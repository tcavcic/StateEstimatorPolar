#pragma once

// =============================================================================
//  StateEstimator.h
// =============================================================================

#include <sparse/ISolver.h>
#include <dp/IDatabase.h>
#include <dp/IDataSet.h>
#include <math/Constants.h>
#include <dense/Matrix.h>
#include <utility>

#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <stdexcept>

struct BusResult
{
    int    id;
    std::string type;
    double V, phi, phi_deg, P, Q;
};

struct BranchResult
{
    int    from, to;
    double Pij, Qij;
    double Pji, Qji;
};

struct MeasResidual
{
    std::string type;
    int         busI;
    int         busJ;
    double      measured;
    double      estimated;
    double      residual;
};

namespace se_detail
{
    struct Bus
    {
        int  id;
        bool isSlack;
    };

    struct Branch
    {
        int    from, to;
        double Y11, Theta11;
        double Y22, Theta22;
        double Y12, Theta12;
        double Y21, Theta21;
    };

    enum class MeasType { P_INJ, Q_INJ, P_FLOW, Q_FLOW, V_MAG };

    struct Measurement
    {
        MeasType type;
        int    i, j;
        double value;
        double weight;
        int    branchIdx = -1;
        bool   reversed  = false;
    };

    struct Trig
    {
        double y, theta, cos_dt, sin_dt;
        Trig(double y_, double theta_, double d)
            : y(y_), theta(theta_),
              cos_dt(std::cos(d - theta_)), sin_dt(std::sin(d - theta_)) {}
        double Pterm() const { return y * cos_dt; }
        double Qterm() const { return y * sin_dt; }
    };
}

class StateEstimator
{
public:
    std::vector<BusResult>    busResults;
    std::vector<BranchResult> branchResults;
    std::vector<MeasResidual> measResiduals;
    std::string               logMsg;
    int                       iterCount  = 0;
    double                    finalError = 0.0;

    bool estimate(dp::IDatabase *pDB, int maxIter = 50, double eps = 1e-6)
    {
        busResults.clear();
        branchResults.clear();
        measResiduals.clear();
        logMsg.clear();
        iterCount  = 0;
        finalError = 0.0;

        if (!loadBuses(pDB))        return false;
        if (!loadBranches(pDB))     return false;
        if (!loadMeasurements(pDB)) return false;
        if (!checkObservability())  return false;

        buildPhiMap();
        buildYbus();
        initStateVector();
        return iterate(maxIter, eps);
    }

private:
    std::vector<se_detail::Bus>         _buses;
    std::vector<se_detail::Branch>      _branches;
    std::vector<se_detail::Measurement> _meas;

    dense::DblMatrix _Y, _Theta;

    std::vector<double> _x;
    std::vector<int>    _phiMap;
    int _N        = 0;
    int _slackIdx = -1;

    int    nx()          const { return (_N - 1) + _N; }
    int    phiIdx(int k) const { return _phiMap[k]; }
    int    vIdx(int k)   const { return (_N - 1) + k; }
    double phi(int k)    const { return (k == _slackIdx) ? 0.0 : _x[phiIdx(k)]; }
    double V(int k)      const { return _x[vIdx(k)]; }

    void buildPhiMap()
    {
        _phiMap.assign(_N, -1);
        int idx = 0;
        for (int k = 0; k < _N; ++k)
            if (k != _slackIdx)
                _phiMap[k] = idx++;
    }

    static void polar2rect(double mag, double deg, double &G, double &B)
    {
        double rad = deg * math::DBL_PI / 180.0;
        G = mag * std::cos(rad);
        B = mag * std::sin(rad);
    }

    static void polar2rectRad(double mag, double rad, double &G, double &B)
    {
        G = mag * std::cos(rad);
        B = mag * std::sin(rad);
    }

    static void rect2polar(double G, double B, double &mag, double &rad)
    {
        mag = std::sqrt(G * G + B * B);
        rad = std::atan2(B, G);
    }

    void addJ(std::vector<std::pair<int,double>> &entries,
              int busK, bool usePhi, double val) const
    {
        if (std::abs(val) < 1e-20) return;
        int col = usePhi ? phiIdx(busK) : vIdx(busK);
        if (col < 0) return;
        entries.emplace_back(col, val);
    }

    bool loadBuses(dp::IDatabase *pDB)
    {
        auto pDS = pDB->createDataSet(
            "SELECT node_id, node_type FROM Nodes ORDER BY node_id",
            dp::IDataSet::Execution::EX_MULT);
        {
            dp::DSColumns c(pDS->allocBindColumns(2));
            c << "node_id"   << td::int4
              << "node_type" << td::string8;
        }
        if (!pDS->execute()) { logMsg = "Ne mogu učitati Nodes!"; return false; }

        _N = (int)pDS->getNumberOfRows();
        if (_N == 0) { logMsg = "Tabela Nodes je prazna!"; return false; }

        _buses.resize(_N);
        _slackIdx = -1;

        for (int k = 0; k < _N; ++k)
        {
            auto &row = pDS->getRow(k);
            _buses[k].id      = row[0].i4Val();
            _buses[k].isSlack = (std::string(row[1].strVal().c_str()) == "SLACK");
            if (_buses[k].isSlack) _slackIdx = k;
        }
        if (_slackIdx < 0) { _slackIdx = 0; _buses[0].isSlack = true; }
        return true;
    }

    bool loadBranches(dp::IDatabase *pDB)
    {
        auto pDS = pDB->createDataSet(
            "SELECT from_node, to_node, "
            "y11_r, y11_phi, y12_r, y12_phi, "
            "y21_r, y21_phi, y22_r, y22_phi, "
            "y0_r,  y0_phi FROM Branches",
            dp::IDataSet::Execution::EX_MULT);
        {
            dp::DSColumns c(pDS->allocBindColumns(12));
            c << "from_node" << td::int4  << "to_node"  << td::int4
              << "y11_r"     << td::real8 << "y11_phi"  << td::real8
              << "y12_r"     << td::real8 << "y12_phi"  << td::real8
              << "y21_r"     << td::real8 << "y21_phi"  << td::real8
              << "y22_r"     << td::real8 << "y22_phi"  << td::real8
              << "y0_r"      << td::real8 << "y0_phi"   << td::real8;
        }
        if (!pDS->execute()) { logMsg = "Ne mogu učitati Branches!"; return false; }

        int nBr = (int)pDS->getNumberOfRows();
        _branches.resize(nBr);

        for (int b = 0; b < nBr; ++b)
        {
            auto &row = pDS->getRow(b);
            auto &br  = _branches[b];

            br.from = busIdx(row[0].i4Val());
            br.to   = busIdx(row[1].i4Val());
            if (br.from < 0 || br.to < 0)
            { logMsg = "Grana referencira nepoznat čvor!"; return false; }

            double y11r = row[2].r8Val(), y11p = row[3].r8Val();
            double y12r = row[4].r8Val(), y12p = row[5].r8Val();
            double y21r = row[6].r8Val(), y21p = row[7].r8Val();
            double y22r = row[8].r8Val(), y22p = row[9].r8Val();
            double y0r  = row[10].r8Val(), y0p = row[11].r8Val();

            br.Y12 = y12r;  br.Theta12 = y12p * math::DBL_PI / 180.0;
            br.Y21 = y21r;  br.Theta21 = y21p * math::DBL_PI / 180.0;

            double G11, B11, G22, B22, G0, B0;
            polar2rect(y11r, y11p, G11, B11);
            polar2rect(y22r, y22p, G22, B22);
            polar2rect(y0r,  y0p,  G0,  B0);

            rect2polar(G11 + G0 / 2.0, B11 + B0 / 2.0, br.Y11, br.Theta11);
            rect2polar(G22 + G0 / 2.0, B22 + B0 / 2.0, br.Y22, br.Theta22);
        }
        return true;
    }

    bool loadMeasurements(dp::IDatabase *pDB)
    {
        const double DEFAULT_SIGMA = 1.0;

        bool hasSigma = false;
        {
            auto pChk = pDB->createDataSet(
                "SELECT sigma FROM Measurements LIMIT 1",
                dp::IDataSet::Execution::EX_MULT);
            dp::DSColumns cc(pChk->allocBindColumns(1));
            cc << "sigma" << td::real8;
            hasSigma = pChk->execute();
        }

        const char *sql = hasSigma
            ? "SELECT node_id, meas_type, meas_index, to_node, value, sigma FROM Measurements ORDER BY id"
            : "SELECT node_id, meas_type, meas_index, to_node, value          FROM Measurements ORDER BY id";

        auto pDS = pDB->createDataSet(sql, dp::IDataSet::Execution::EX_MULT);
        if (hasSigma)
        {
            dp::DSColumns c(pDS->allocBindColumns(6));
            c << "node_id"    << td::int4  << "meas_type"  << td::string8
              << "meas_index" << td::int4  << "to_node"    << td::int4
              << "value"      << td::real8 << "sigma"      << td::real8;
        }
        else
        {
            dp::DSColumns c(pDS->allocBindColumns(5));
            c << "node_id"    << td::int4  << "meas_type"  << td::string8
              << "meas_index" << td::int4  << "to_node"    << td::int4
              << "value"      << td::real8;
        }
        if (!pDS->execute()) { logMsg = "Ne mogu učitati Measurements!"; return false; }

        int M = (int)pDS->getNumberOfRows();
        _meas.reserve(M);

        using MT = se_detail::MeasType;
        for (int m = 0; m < M; ++m)
        {
            auto &row = pDS->getRow(m);
            std::string ts    = row[1].strVal().c_str();
            int    fromID     = row[0].i4Val();
            int    toID       = row[3].i4Val();
            double val        = row[4].r8Val();
            double sigma      = hasSigma ? row[5].r8Val() : DEFAULT_SIGMA;
            if (sigma <= 0.0) sigma = DEFAULT_SIGMA;

            int fi = busIdx(fromID);
            if (fi < 0) { logMsg = "Mjerenje referencira nepoznat čvor!"; return false; }

            MT type;
            if      (ts == "P_FLOW")             type = MT::P_FLOW;
            else if (ts == "Q_FLOW")             type = MT::Q_FLOW;
            else if (ts == "V_MAG" || ts == "V") type = MT::V_MAG;
            else if (ts == "P_INJ" || ts == "P") type = MT::P_INJ;
            else if (ts == "Q_INJ" || ts == "Q") type = MT::Q_INJ;
            else if (ts == "delta" && fi == _slackIdx) continue;
            else { logMsg = "Nepoznat tip mjerenja: " + ts; return false; }

            int ti = (type == MT::P_FLOW || type == MT::Q_FLOW) ? busIdx(toID) : -1;

            int  brIdx    = -1;
            bool reversed = false;
            if (type == MT::P_FLOW || type == MT::Q_FLOW)
            {
                if (ti < 0) { logMsg = "P/Q_FLOW mjerenje bez validnog 'to' cvora!"; return false; }
                for (size_t bi = 0; bi < _branches.size(); ++bi)
                {
                    if (_branches[bi].from == fi && _branches[bi].to == ti)
                    { brIdx = (int)bi; reversed = false; break; }
                    if (_branches[bi].from == ti && _branches[bi].to == fi)
                    { brIdx = (int)bi; reversed = true; break; }
                }
                if (brIdx < 0)
                {
                    logMsg = "P/Q_FLOW mjerenje (" + std::to_string(fromID) + "->"
                           + std::to_string(toID) + ") ne odgovara nijednoj grani!";
                    return false;
                }
            }

            se_detail::Measurement meas;
            meas.type      = type;
            meas.i         = fi;
            meas.j         = ti;
            meas.value     = val;
            meas.weight    = 1.0 / (sigma * sigma);
            meas.branchIdx = brIdx;
            meas.reversed  = reversed;
            _meas.push_back(meas);
        }
        return true;
    }

    bool checkObservability()
    {
        int have = (int)_meas.size(), need = nx();
        if (have < need)
        {
            logMsg = "Nedovoljno mjerenja: ima " + std::to_string(have)
                   + ", treba najmanje " + std::to_string(need) + ".";
            return false;
        }
        return true;
    }

    void buildYbus()
    {
        std::vector<double> Gt(_N * _N, 0.0), Bt(_N * _N, 0.0);

        for (auto &br : _branches)
        {
            int i = br.from, j = br.to;

            double G11, B11, G22, B22, G12, B12, G21, B21;
            polar2rectRad(br.Y11, br.Theta11, G11, B11);
            polar2rectRad(br.Y22, br.Theta22, G22, B22);
            polar2rectRad(br.Y12, br.Theta12, G12, B12);
            polar2rectRad(br.Y21, br.Theta21, G21, B21);

            Gt[i*_N+i] += G11;  Bt[i*_N+i] += B11;
            Gt[j*_N+j] += G22;  Bt[j*_N+j] += B22;
            Gt[i*_N+j] += G12;  Bt[i*_N+j] += B12;
            Gt[j*_N+i] += G21;  Bt[j*_N+i] += B21;
        }

        _Y     = dense::DblMatrix(_N, _N, nullptr, true);
        _Theta = dense::DblMatrix(_N, _N, nullptr, true);

        auto ioY  = _Y.getManipulator();
        auto ioTh = _Theta.getManipulator();
        for (int i = 0; i < _N; ++i)
            for (int j = 0; j < _N; ++j)
            {
                double mag, rad;
                rect2polar(Gt[i*_N+j], Bt[i*_N+j], mag, rad);
                ioY(i, j)  = mag;
                ioTh(i, j) = rad;
            }
    }

    void initStateVector()
    {
        _x.assign(nx(), 0.0);
        for (int k = 0; k < _N; ++k) _x[vIdx(k)] = 1.0;
    }

    bool iterate(int maxIter, double eps)
    {
        int    M      = (int)_meas.size();
        int    nxSize = nx();
        double maxDx  = 1.0;

        std::vector<std::pair<int,double>> rowEntries;
        rowEntries.reserve(16);

        for (int iter = 1; iter <= maxIter && maxDx > eps; ++iter)
        {
            sparse::DblSolverReleaser slvRel(
                sparse::createDblSolver(
                    nxSize,
                    nxSize * nxSize,
                    sparse::Symmetry::SymmetricPosDef,
                    sparse::SolverType::LDLT,
                    sparse::Pivoting::DiagonalSinglePass));

            if (!slvRel.ptr())
            {
                logMsg = "createDblSolver vratio nullptr!";
                return false;
            }
            sparse::DblSolver &slv = *slvRel.ptr();

            slv.populateDiagonals(0.0);
            slv.clearRHS();

            for (int m = 0; m < M; ++m)
            {
                rowEntries.clear();
                double h = computeH(m, rowEntries);
                double r = h - _meas[m].value;
                double w = _meas[m].weight;

                for (const auto &a : rowEntries)
                    slv.RHS(a.first) += w * a.second * r;

                for (size_t a = 0; a < rowEntries.size(); ++a)
                {
                    int    colA = rowEntries[a].first;
                    double valA = rowEntries[a].second;
                    for (size_t b = a; b < rowEntries.size(); ++b)
                    {
                        int    colB = rowEntries[b].first;
                        double valB = rowEntries[b].second;
                        int    lo   = std::min(colA, colB);
                        int    hi   = std::max(colA, colB);
                        slv.addTriple(lo, hi, w * valA * valB);
                    }
                }
            }

            if (!slv.factorize())
            {
                logMsg = "Faktorizacija nije uspjela u iteraciji " + std::to_string(iter);
                return false;
            }
            if (!slv.solve())
            {
                logMsg = "Solve nije uspio u iteraciji " + std::to_string(iter);
                return false;
            }

            maxDx = 0.0;
            for (int i = 0; i < nxSize; ++i)
            {
                double dx = slv.x(i);
                _x[i] -= dx;
                maxDx = std::max(maxDx, std::abs(dx));
            }

            iterCount  = iter;
            finalError = maxDx;
            appendIterLog(iter, maxDx);
        }

        bool converged = (maxDx <= eps);
        extractResults();
        extractResiduals();
        logMsg = "Konvergencija: " + std::string(converged ? "DA" : "NE")
               + "  maxDx=" + std::to_string(maxDx) + "\n" + logMsg;
        return converged;
    }

    static void flowSelfMutual(const se_detail::Branch &br, bool reversed,
                                double &Yself, double &Thself,
                                double &Ymut,  double &Thmut)
    {
        if (!reversed) { Yself = br.Y11; Thself = br.Theta11; Ymut = br.Y12; Thmut = br.Theta12; }
        else           { Yself = br.Y22; Thself = br.Theta22; Ymut = br.Y21; Thmut = br.Theta21; }
    }

    double computeH(int m, std::vector<std::pair<int,double>> &entries) const
    {
        using MT   = se_detail::MeasType;
        using Trig = se_detail::Trig;
        const auto &meas = _meas[m];
        int i = meas.i;

        switch (meas.type)
        {
        case MT::P_INJ:
        {
            auto rY  = _Y.getManipulator();
            auto rTh = _Theta.getManipulator();

            double Vi = V(i), pi = phi(i), h = 0.0, dPdphiI = 0.0;
            for (int l = 0; l < _N; ++l)
            {
                Trig t(rY(i, l), rTh(i, l), pi - phi(l));
                double Vl = V(l);
                h += Vl * t.Pterm();
                if (l != i)
                {
                    dPdphiI += Vl * (-t.Qterm());
                    addJ(entries, l, true,   Vi * Vl * t.Qterm());
                    addJ(entries, l, false,  Vi * t.Pterm());
                }
            }
            h *= Vi;
            double Gself_i = rY(i, i) * std::cos(rTh(i, i));
            addJ(entries, i, true,  Vi * dPdphiI);
            addJ(entries, i, false, h / Vi + Vi * Gself_i);
            return h;
        }

        case MT::Q_INJ:
        {
            auto rY  = _Y.getManipulator();
            auto rTh = _Theta.getManipulator();

            double Vi = V(i), pi = phi(i), h = 0.0, dQdphiI = 0.0;
            for (int l = 0; l < _N; ++l)
            {
                Trig t(rY(i, l), rTh(i, l), pi - phi(l));
                double Vl = V(l);
                h += Vl * t.Qterm();
                if (l != i)
                {
                    dQdphiI += Vl * t.Pterm();
                    addJ(entries, l, true,  -Vi * Vl * t.Pterm());
                    addJ(entries, l, false,  Vi * t.Qterm());
                }
            }
            h *= Vi;
            double Bself_i = rY(i, i) * std::sin(rTh(i, i));
            addJ(entries, i, true,  Vi * dQdphiI);
            addJ(entries, i, false, h / Vi - Vi * Bself_i);
            return h;
        }

        case MT::P_FLOW:
        {
            int j = meas.j;
            double Vi = V(i), Vj = V(j);
            const auto &br = _branches[meas.branchIdx];
            double Yself, Thself, Ymut, Thmut;
            flowSelfMutual(br, meas.reversed, Yself, Thself, Ymut, Thmut);
            double d = phi(i) - phi(j);
            Trig t(Ymut, Thmut, d);
            double Gself = Yself * std::cos(Thself);
            double h = Vi*Vi*Gself + Vi*Vj*t.Pterm();

            addJ(entries, i, true,  -Vi*Vj*t.Qterm());
            addJ(entries, j, true,   Vi*Vj*t.Qterm());
            addJ(entries, i, false,  2*Vi*Gself + Vj*t.Pterm());
            addJ(entries, j, false,  Vi*t.Pterm());
            return h;
        }

        case MT::Q_FLOW:
        {
            int j = meas.j;
            double Vi = V(i), Vj = V(j);
            const auto &br = _branches[meas.branchIdx];
            double Yself, Thself, Ymut, Thmut;
            flowSelfMutual(br, meas.reversed, Yself, Thself, Ymut, Thmut);
            double d = phi(i) - phi(j);
            Trig t(Ymut, Thmut, d);
            double Bself = Yself * std::sin(Thself);
            double h = -Vi*Vi*Bself + Vi*Vj*t.Qterm();

            addJ(entries, i, true,   Vi*Vj*t.Pterm());
            addJ(entries, j, true,  -Vi*Vj*t.Pterm());
            addJ(entries, i, false, -2*Vi*Bself + Vj*t.Qterm());
            addJ(entries, j, false,  Vi*t.Qterm());
            return h;
        }

        case MT::V_MAG:
            addJ(entries, i, false, 1.0);
            return V(i);

        default:
            return 0.0;
        }
    }

    void extractResults()
    {
        busResults.resize(_N);

        auto rY  = _Y.getManipulator();
        auto rTh = _Theta.getManipulator();
        for (int k = 0; k < _N; ++k)
        {
            double Vk = V(k), pk = phi(k), Pi = 0.0, Qi = 0.0;
            for (int l = 0; l < _N; ++l)
            {
                se_detail::Trig t(rY(k, l), rTh(k, l), pk - phi(l));
                double Vl = V(l);
                Pi += Vl * t.Pterm();
                Qi += Vl * t.Qterm();
            }
            busResults[k] = {_buses[k].id,
                             _buses[k].isSlack ? "SLACK" : "PQ",
                             Vk, pk, pk * 180.0 / math::DBL_PI,
                             Vk * Pi, Vk * Qi};
        }

        branchResults.resize(_branches.size());
        for (int b = 0; b < (int)_branches.size(); ++b)
        {
            const auto &br = _branches[b];
            int    i = br.from, j = br.to;
            double Vi = V(i), Vj = V(j);

            se_detail::Trig tij(br.Y12, br.Theta12, phi(i) - phi(j));
            double Gself_i = br.Y11 * std::cos(br.Theta11);
            double Bself_i = br.Y11 * std::sin(br.Theta11);
            double Pij =  Vi*Vi*Gself_i + Vi*Vj*tij.Pterm();
            double Qij = -Vi*Vi*Bself_i + Vi*Vj*tij.Qterm();

            se_detail::Trig tji(br.Y21, br.Theta21, phi(j) - phi(i));
            double Gself_j = br.Y22 * std::cos(br.Theta22);
            double Bself_j = br.Y22 * std::sin(br.Theta22);
            double Pji =  Vj*Vj*Gself_j + Vj*Vi*tji.Pterm();
            double Qji = -Vj*Vj*Bself_j + Vj*Vi*tji.Qterm();

            branchResults[b] = {_buses[i].id, _buses[j].id, Pij, Qij, Pji, Qji};
        }
    }

    void extractResiduals()
    {
        using MT = se_detail::MeasType;
        static const char* typeNames[] = {"P_INJ","Q_INJ","P_FLOW","Q_FLOW","V_MAG"};

        measResiduals.resize(_meas.size());
        for (int m = 0; m < (int)_meas.size(); ++m)
        {
            const auto &meas = _meas[m];
            double hx = computeHonly(m);

            int typeIdx = (int)meas.type;
            std::string tn = typeNames[typeIdx];

            int busIdI = _buses[meas.i].id;
            int busIdJ = (meas.j >= 0) ? _buses[meas.j].id : -1;

            measResiduals[m] = {
                tn,
                busIdI,
                busIdJ,
                meas.value,
                hx,
                hx - meas.value
            };
        }
    }

    double computeHonly(int m) const
    {
        using MT   = se_detail::MeasType;
        using Trig = se_detail::Trig;
        const auto &meas = _meas[m];
        int i = meas.i;

        switch (meas.type)
        {
        case MT::P_INJ:
        {
            auto rY  = _Y.getManipulator();
            auto rTh = _Theta.getManipulator();
            double Vi = V(i), pi = phi(i), h = 0.0;
            for (int l = 0; l < _N; ++l)
            {
                Trig t(rY(i, l), rTh(i, l), pi - phi(l));
                h += V(l) * t.Pterm();
            }
            return Vi * h;
        }
        case MT::Q_INJ:
        {
            auto rY  = _Y.getManipulator();
            auto rTh = _Theta.getManipulator();
            double Vi = V(i), pi = phi(i), h = 0.0;
            for (int l = 0; l < _N; ++l)
            {
                Trig t(rY(i, l), rTh(i, l), pi - phi(l));
                h += V(l) * t.Qterm();
            }
            return Vi * h;
        }
        case MT::P_FLOW:
        {
            int j = meas.j; double Vi=V(i), Vj=V(j);
            const auto &br = _branches[meas.branchIdx];
            double Yself, Thself, Ymut, Thmut;
            flowSelfMutual(br, meas.reversed, Yself, Thself, Ymut, Thmut);
            Trig t(Ymut, Thmut, phi(i) - phi(j));
            double Gself = Yself * std::cos(Thself);
            return Vi*Vi*Gself + Vi*Vj*t.Pterm();
        }
        case MT::Q_FLOW:
        {
            int j = meas.j; double Vi=V(i), Vj=V(j);
            const auto &br = _branches[meas.branchIdx];
            double Yself, Thself, Ymut, Thmut;
            flowSelfMutual(br, meas.reversed, Yself, Thself, Ymut, Thmut);
            Trig t(Ymut, Thmut, phi(i) - phi(j));
            double Bself = Yself * std::sin(Thself);
            return -Vi*Vi*Bself + Vi*Vj*t.Qterm();
        }
        case MT::V_MAG:
            return V(i);
        default:
            return 0.0;
        }
    }

    void appendIterLog(int iter, double maxDx)
    {
        logMsg = "Iter " + std::to_string(iter)
               + "  maxDx=" + std::to_string(maxDx) + "\n" + logMsg;
    }

    int busIdx(int id) const
    {
        for (int k = 0; k < _N; ++k)
            if (_buses[k].id == id) return k;
        return -1;
    }
};