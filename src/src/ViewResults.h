#pragma once
#include <gui/View.h>
#include <gui/Label.h>
#include <gui/TableEdit.h>
#include <gui/TextEdit.h>
#include <gui/Button.h>
#include <gui/GridLayout.h>
#include <gui/GridComposer.h>
#include <gui/StandardTabView.h>
#include <dp/IDatabase.h>
#include <dp/IDataSet.h>
#include <fo/FileOperations.h>
#include <vector>
#include <string>
#include "StateEstimator.h"

// =============================================================================
//  ViewResults.h
// =============================================================================

struct MeasResult {
    std::string type;
    int         from_bus;
    int         to_bus;
    double      measured;
    double      estimated;
    double      residual;
};

struct EstimationResults {
    bool                      converged    = false;
    bool                      observable   = false;
    int                       iterations   = 0;
    double                    finalError   = 0.0;
    std::string               errorMsg;
    std::vector<BusResult>    buses;
    std::vector<BranchResult> branches;
    std::vector<MeasResult>   measurements;
};
class BusResultsView : public gui::View
{
    gui::Label      _lbl;
    gui::TableEdit  _table;
    gui::GridLayout _gl;

    dp::IDatabase*  _db    = nullptr;   
    dp::IDataSetPtr _pDS;
    bool            _ready = false;

public:
    BusResultsView()
    : _lbl("Stanja čvorova  (V, φ, P, Q)")
    , _gl(2, 1)
    {
        gui::GridComposer gc(_gl);
        gc.appendRow(_lbl);
        gc.appendRow(_table, 0);
        setLayout(&_gl);
    }

    void init(dp::IDatabase* pDB)
    {
        _db = pDB;

        dp::IStatementPtr s(_db->createStatement(
            "CREATE TABLE IF NOT EXISTS BusResults ("
            "node_id INTEGER, node_type TEXT, "
            "V REAL, phi_deg REAL, P REAL, Q REAL)"));
        s->execute();

        _pDS = _db->createDataSet(
            "SELECT node_id, node_type, V, phi_deg, P, Q "
            "FROM BusResults ORDER BY node_id",
            dp::IDataSet::Execution::EX_MULT);
        {
            dp::DSColumns cols(_pDS->allocBindColumns(6));
            cols << "node_id"   << td::int4
                 << "node_type" << td::string8
                 << "V"         << td::real8
                 << "phi_deg"   << td::real8
                 << "P"         << td::real8
                 << "Q"         << td::real8;
        }
        _pDS->execute();

        gui::Columns vc(_table.allocBindColumns(6));
        vc << gui::Header(0, "ID",       "ID",      50,  td::HAlignment::Center)
           << gui::Header(1, "Tip",      "Tip",     60,  td::HAlignment::Center)
           << gui::Header(2, "V [p.u.]", "V",      100,  td::HAlignment::Center)
           << gui::Header(3, "φ [°]",    "phi",     90,  td::HAlignment::Center)
           << gui::Header(4, "P [pu]",   "P",      100,  td::HAlignment::Center)
           << gui::Header(5, "Q [pu]",   "Q",      100,  td::HAlignment::Center);
        _table.init(_pDS);
        for (int c = 2; c <= 5; ++c)
            _table.setColumnNumericFormat(c, td::FormatFloat::Decimal, 6);

        _ready = true;
    }

    void refresh(const std::vector<BusResult>& buses)
    {
        if (!_ready) return;

        {
            dp::IStatementPtr pDel(_db->createStatement("DELETE FROM BusResults"));
            pDel->execute();
        }
        {
            dp::IStatementPtr pIns(_db->createStatement(
                "INSERT INTO BusResults (node_id, node_type, V, phi_deg, P, Q) "
                "VALUES (?, ?, ?, ?, ?, ?)"));
            for (const auto& b : buses)
            {
                dp::Params par(pIns->allocParams());
                td::Variant id(b.id);
                td::Variant typeVar(td::string8, td::nch, 16);
                td::String  typeStr(b.type.c_str());
                typeVar.setValue(typeStr);
                td::Variant V(td::real8);      V      = b.V;
                td::Variant phiDeg(td::real8); phiDeg = b.phi_deg;
                td::Variant P(td::real8);      P      = b.P;
                td::Variant Q(td::real8);      Q      = b.Q;
                par << id << typeVar << V << phiDeg << P << Q;
                pIns->execute();
            }
        }

        _table.reload();
    }
};

class BranchResultsView : public gui::View
{
    gui::Label      _lbl;
    gui::TableEdit  _table;
    gui::GridLayout _gl;

    dp::IDatabase*  _db    = nullptr;
    dp::IDataSetPtr _pDS;
    bool            _ready = false;

public:
    BranchResultsView()
    : _lbl("Tokovi snage u granama")
    , _gl(2, 1)
    {
        gui::GridComposer gc(_gl);
        gc.appendRow(_lbl);
        gc.appendRow(_table, 0);
        setLayout(&_gl);
    }

    void init(dp::IDatabase* pDB)
    {
        _db = pDB;

        dp::IStatementPtr s(_db->createStatement(
            "CREATE TABLE IF NOT EXISTS BranchResults ("
            "from_bus INTEGER, to_bus INTEGER, "
            "Pij REAL, Qij REAL, Pji REAL, Qji REAL)"));
        s->execute();

        _pDS = _db->createDataSet(
            "SELECT from_bus, to_bus, Pij, Qij, Pji, Qji FROM BranchResults",
            dp::IDataSet::Execution::EX_MULT);
        {
            dp::DSColumns cols(_pDS->allocBindColumns(6));
            cols << "from_bus" << td::int4
                 << "to_bus"   << td::int4
                 << "Pij"      << td::real8
                 << "Qij"      << td::real8
                 << "Pji"      << td::real8
                 << "Qji"      << td::real8;
        }
        _pDS->execute();

        gui::Columns vc(_table.allocBindColumns(6));
        vc << gui::Header(0, "Od",        "Od",    55,  td::HAlignment::Center)
           << gui::Header(1, "Do",        "Do",    55,  td::HAlignment::Center)
           << gui::Header(2, "P_ij [pu]", "Pij",  110,  td::HAlignment::Center)
           << gui::Header(3, "Q_ij [pu]", "Qij",  110,  td::HAlignment::Center)
           << gui::Header(4, "P_ji [pu]", "Pji",  110,  td::HAlignment::Center)
           << gui::Header(5, "Q_ji [pu]", "Qji",  110,  td::HAlignment::Center);
        _table.init(_pDS);
        for (int c = 2; c <= 5; ++c)
            _table.setColumnNumericFormat(c, td::FormatFloat::Decimal, 6);

        _ready = true;
    }

    void refresh(const std::vector<BranchResult>& branches)
    {
        if (!_ready) return;

        {
            dp::IStatementPtr pDel(_db->createStatement("DELETE FROM BranchResults"));
            pDel->execute();
        }
        {
            dp::IStatementPtr pIns(_db->createStatement(
                "INSERT INTO BranchResults (from_bus, to_bus, Pij, Qij, Pji, Qji) "
                "VALUES (?, ?, ?, ?, ?, ?)"));
            for (const auto& br : branches)
            {
                dp::Params par(pIns->allocParams());
                td::Variant from(br.from);
                td::Variant to(br.to);
                td::Variant Pij(td::real8); Pij = br.Pij;
                td::Variant Qij(td::real8); Qij = br.Qij;
                td::Variant Pji(td::real8); Pji = br.Pji;
                td::Variant Qji(td::real8); Qji = br.Qji;
                par << from << to << Pij << Qij << Pji << Qji;
                pIns->execute();
            }
        }

        _table.reload();
    }
};

class MeasResultsView : public gui::View
{
    gui::Label      _lbl;
    gui::TableEdit  _table;
    gui::GridLayout _gl;

    dp::IDatabase*  _db    = nullptr;
    dp::IDataSetPtr _pDS;
    bool            _ready = false;

public:
    MeasResultsView()
    : _lbl("Reziduali mjerenja")
    , _gl(2, 1)
    {
        gui::GridComposer gc(_gl);
        gc.appendRow(_lbl);
        gc.appendRow(_table, 0);
        setLayout(&_gl);
    }

    void init(dp::IDatabase* pDB)
    {
        _db = pDB;

        dp::IStatementPtr s(_db->createStatement(
            "CREATE TABLE IF NOT EXISTS MeasResults ("
            "meas_type TEXT, from_bus INTEGER, to_bus INTEGER, "
            "measured REAL, estimated REAL, residual REAL)"));
        s->execute();

        _pDS = _db->createDataSet(
            "SELECT meas_type, from_bus, to_bus, measured, estimated, residual "
            "FROM MeasResults",
            dp::IDataSet::Execution::EX_MULT);
        {
            dp::DSColumns cols(_pDS->allocBindColumns(6));
            cols << "meas_type" << td::string8
                 << "from_bus"  << td::int4
                 << "to_bus"    << td::int4
                 << "measured"  << td::real8
                 << "estimated" << td::real8
                 << "residual"  << td::real8;
        }
        _pDS->execute();

        gui::Columns vc(_table.allocBindColumns(6));
        vc << gui::Header(0, "Tip",       "Tip",    90,  td::HAlignment::Center)
           << gui::Header(1, "Čvor",      "Cvor",   60,  td::HAlignment::Center)
           << gui::Header(2, "Susjedni",  "Sus.",   70,  td::HAlignment::Center)
           << gui::Header(3, "Izmjereno", "Izmj.", 100,  td::HAlignment::Center)
           << gui::Header(4, "h(x)",      "hx",    100,  td::HAlignment::Center)
           << gui::Header(5, "Rezidual",  "Rez.",  100,  td::HAlignment::Center);
        _table.init(_pDS);
        for (int c = 3; c <= 5; ++c)
            _table.setColumnNumericFormat(c, td::FormatFloat::Decimal, 6);

        _ready = true;
    }

    void refresh(const std::vector<MeasResult>& meas)
    {
        if (!_ready) return;

        {
            dp::IStatementPtr pDel(_db->createStatement("DELETE FROM MeasResults"));
            pDel->execute();
        }
        {
            dp::IStatementPtr pIns(_db->createStatement(
                "INSERT INTO MeasResults "
                "(meas_type, from_bus, to_bus, measured, estimated, residual) "
                "VALUES (?, ?, ?, ?, ?, ?)"));
            for (const auto& m : meas)
            {
                dp::Params par(pIns->allocParams());
                td::Variant typeVar(td::string8, td::nch, 16);
                td::String  typeStr(m.type.c_str());
                typeVar.setValue(typeStr);
                td::Variant fromBus(m.from_bus);
                td::Variant toBus(m.to_bus);
                td::Variant measured(td::real8);  measured  = m.measured;
                td::Variant estimated(td::real8); estimated = m.estimated;
                td::Variant residual(td::real8);  residual  = m.residual;
                par << typeVar << fromBus << toBus << measured << estimated << residual;
                pIns->execute();
            }
        }

        _table.reload();
    }
};

class ViewResults : public gui::View
{
    gui::Label           _lblStatus;
    gui::TextEdit        _statusLog;
    gui::StandardTabView _tabs;
    BusResultsView       _busView;
    BranchResultsView    _branchView;
    MeasResultsView      _measView;
    gui::GridLayout      _gl;

public:
    ViewResults()
    : _lblStatus("Status estimacije:")
    , _gl(3, 1)
    {
        _statusLog.setAsReadOnly();

        _tabs.addView(&_busView,    "Stanja čvorova",   nullptr);
        _tabs.addView(&_branchView, "Tokovi u granama", nullptr);
        _tabs.addView(&_measView,   "Reziduali",        nullptr);

        gui::GridComposer gc(_gl);
        gc.appendRow(_lblStatus);
        gc.appendRow(_statusLog);
        gc.appendRow(_tabs, 0);
        setLayout(&_gl);

        _statusLog.appendString("Pritisnite 'Estimiraj Stanje' za pokretanje.\n");
    }

    void setDatabase(dp::IDatabase* pDB)
    {
        _busView.init(pDB);
        _branchView.init(pDB);
        _measView.init(pDB);
    }

    void showResults(const EstimationResults& res)
    {
        _statusLog.clean();

        if (!res.observable)
        {
            td::String msg;
            msg.format("GREŠKA: Sistem nije observabilan!\n%s\n",
                       res.errorMsg.c_str());
            _statusLog.appendString(msg);
            return;
        }

        if (!res.converged)
        {
            td::String msg;
            msg.format("UPOZORENJE: Nije dostigla konvergencija (%d iter)!\n"
                       "Finalna greška: %.2e\n%s\n",
                       res.iterations, res.finalError, res.errorMsg.c_str());
            _statusLog.appendString(msg);
        }
        else
        {
            td::String msg;
            msg.format("OK: Konvergencija za %d iteracija.  Finalna greška: %.2e\n"
                       "Čvorova: %zu  |  Grana: %zu  |  Mjerenja: %zu\n"
                       "─────────────────────────────────────────\n%s",
                       res.iterations, res.finalError,
                       res.buses.size(), res.branches.size(),
                       res.measurements.size(),
                       res.errorMsg.c_str());
            _statusLog.appendString(msg);
        }

        _busView.refresh(res.buses);
        _branchView.refresh(res.branches);
        _measView.refresh(res.measurements);
    }
};