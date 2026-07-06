#pragma once
#include <gui/natID.h>
#include <gui/TableEdit.h>
#include <gui/GridComposer.h>
#include <dp/IDatabase.h>
#include <dp/IDataSet.h>
#include <fo/FileOperations.h>
#include "StateEstimator.h"
#include "ViewResults.h"

// =============================================================================
//  ViewMeas.h
// =============================================================================

class MeasInputView : public gui::View
{
public:
    gui::Label       _lblNodeID;
    gui::ComboBox    _cbNodeID;

    gui::Label       _lblMeasType;
    gui::ComboBox    _cbMeasType;   

    gui::Label       _lblToNode;
    gui::ComboBox    _cbToNode;     

    gui::Label       _lblValue;
    gui::NumericEdit _valueEdit;

    gui::Label       _lblSigma;
    gui::NumericEdit _sigmaEdit;

    gui::Button      _btnAddMeas;
    gui::GridLayout  _glInput;

    MeasInputView()
    : _lblNodeID("Čvor:")
    , _lblMeasType("Tip mjerenja:")
    , _lblToNode("Do čvora:")
    , _lblValue("Vrijednost:")
    , _valueEdit(td::real8)
    , _lblSigma("Sigma:")
    , _sigmaEdit(td::real8)
    , _btnAddMeas("Dodaj mjerenje")
    , _glInput(6, 2)
    {
        _cbMeasType.addItem("P (injekcija)");
        _cbMeasType.addItem("Q (injekcija)");
        _cbMeasType.addItem("|V| (napon)");
        _cbMeasType.addItem("P (tok snage)");
        _cbMeasType.addItem("Q (tok snage)");
        _cbMeasType.selectIndex(0);
        td::Variant defSigma(1.0);
        _sigmaEdit.setValue(defSigma);
        gui::GridComposer gc(_glInput);
        gc.appendRow(_lblNodeID);
        gc.appendCol(_cbNodeID);
        gc.appendRow(_lblMeasType);
        gc.appendCol(_cbMeasType);
        gc.appendRow(_lblToNode);
        gc.appendCol(_cbToNode);
        gc.appendRow(_lblValue);
        gc.appendCol(_valueEdit);
        gc.appendRow(_lblSigma);
        gc.appendCol(_sigmaEdit);
        gc.appendRow(_btnAddMeas, 0);
        setLayout(&_glInput);
        _cbToNode.enable(false);
    }
};

class ViewMeas : public gui::View
{
protected:
    MeasInputView    _inputView;
    gui::TextEdit    _log;

    gui::Label       _lblMeasurements;
    gui::TableEdit   _table;
    gui::Button      _btnDelete;
    gui::Button      _btnReload;

    gui::GridLayout  _gl;

    dp::IDatabasePtr _db;
    dp::IDataSetPtr  _pDS;
    ViewResults*     _pResults = nullptr;

public:
    ViewMeas()
    : _lblMeasurements("Mjerenja:")
    , _btnDelete("Obrisi mjerenje")
    , _btnReload("Osvjezi")
    , _gl(6, 1)
    , _db(dp::create(dp::IDatabase::ConnType::CT_SQLITE,
                     dp::IDatabase::ServerType::SER_SQLITE3))
    {
        _log.setAsReadOnly();
        _inputView._btnAddMeas.setAsDefault();
        _inputView._btnAddMeas.forwardMessagesTo(this);
        _inputView._cbMeasType.forwardMessagesTo(this);
        _btnDelete.forwardMessagesTo(this);
        _btnReload.forwardMessagesTo(this);

        gui::GridComposer gc(_gl);
        gc.appendRow(_inputView);
        gc.appendRow(_log, 0);
        gc.appendRow(_lblMeasurements, 0);
        gc.appendRow(_table, 0);
        gc.appendRow(_btnDelete, 0);
        gc.appendRow(_btnReload, 0);
        setLayout(&_gl);

        initDB();
        populateData();
    }
    void setResultsView(ViewResults* pResults)
    {
        _pResults = pResults;
        if (_pResults && _db.ptr())
            _pResults->setDatabase(_db.ptr());
    }

    void setNodeList(const std::vector<std::pair<int, td::String>>& nodes)
    {
        _inputView._cbNodeID.clean();
        _inputView._cbToNode.clean();
        for (const auto& n : nodes)
        {
            td::String item;
            item.format("Čvor %d (%s)", n.first, n.second.c_str());
            _inputView._cbNodeID.addItem(item);
            _inputView._cbToNode.addItem(item);
        }
        if (!nodes.empty())
        {
            _inputView._cbNodeID.selectIndex(0);
            _inputView._cbToNode.selectIndex(nodes.size() > 1 ? 1 : 0);
        }
    }

    void estimateState()
    {
        if (!_db.ptr())
        {
            showAlert(td::String("Greška"), td::String("Baza nije inicijalizovana!"));
            return;
        }

        EstimationResults res;

        try
        {
            StateEstimator se;
            bool ok = se.estimate(_db.ptr());

            res.converged  = ok;
            res.observable = true;
            res.iterations = se.iterCount;
            res.finalError = se.finalError;
            res.errorMsg   = se.logMsg;

            for (const auto& b : se.busResults)
            {
                BusResult br;
                br.id = b.id; br.type = b.type;
                br.V = b.V; br.phi = b.phi; br.phi_deg = b.phi_deg;
                br.P = b.P; br.Q = b.Q;
                res.buses.push_back(br);
            }
            for (const auto& b : se.branchResults)
            {
                BranchResult bres;
                bres.from = b.from; bres.to = b.to;
                bres.Pij = b.Pij; bres.Qij = b.Qij;
                bres.Pji = b.Pji; bres.Qji = b.Qji;
                res.branches.push_back(bres);
            }
            for (const auto& m : se.measResiduals)
            {
                MeasResult mr;
                mr.type = m.type;
                mr.from_bus = m.busI; mr.to_bus = m.busJ;
                mr.measured = m.measured; mr.estimated = m.estimated;
                mr.residual = m.residual;
                res.measurements.push_back(mr);
            }
        }
        catch (const std::exception& e)
        {
            std::string msg = e.what();
            res.converged  = false;
            res.observable = (msg.find("observabilan") == std::string::npos);
            res.errorMsg   = msg;
            td::String fmtMsg;
            fmtMsg.format("GREŠKA: %s\n", msg.c_str());
            _log.appendString(fmtMsg);
        }
        catch (...)
        {
            res.converged = false; res.observable = false;
            res.errorMsg  = "Nepoznata greška!";
            _log.appendString("GREŠKA: Nepoznata greška!\n");
        }

        td::String msg;
        msg.format("[Estimacija] iter=%d err=%.2e conv=%s\n",
                   res.iterations, res.finalError,
                   res.converged ? "DA" : "NE");
        _log.appendString(msg);

        if (_pResults)
            _pResults->showResults(res);
    }

protected:
    void initDB()
    {
        td::String dbFileName = gui::getResFileName(":Baza.db");
        if (!_db->connect(dbFileName))
        {
            showAlert("DB error", "Ne mogu otvoriti Baza.db");
            return;
        }
        _log.appendString("Konekcija na bazu uspostavljena.\n");

        if (_pResults)
            _pResults->setDatabase(_db.ptr());
    }
    bool populateData()
    {
        _pDS = _db->createDataSet(
            "SELECT "
            "  m.id, "
            "  m.node_id, "
            "  n.node_type, "
            "  CASE m.meas_type "
            "    WHEN 'P_INJ'  THEN 'P (inj.)' "
            "    WHEN 'Q_INJ'  THEN 'Q (inj.)' "
            "    WHEN 'V_MAG'  THEN '|V|' "
            "    WHEN 'P_FLOW' THEN 'P (tok)' "
            "    WHEN 'Q_FLOW' THEN 'Q (tok)' "
            "    ELSE m.meas_type "
            "  END AS meas_type_disp, "
            "  m.to_node, "
            "  m.value, "
            "  m.sigma "
            "FROM Measurements m "
            "JOIN Nodes n ON n.node_id = m.node_id "
            "ORDER BY m.id",
            dp::IDataSet::Execution::EX_MULT);

        dp::DSColumns cols(_pDS->allocBindColumns(7));
        cols << "id"             << td::int4
             << "node_id"        << td::int4
             << "node_type"      << td::string8
             << "meas_type_disp" << td::string8
             << "to_node"        << td::int4
             << "value"          << td::real8
             << "sigma"          << td::real8;

        if (!_pDS->execute())
            return false;

        gui::Columns visCols(_table.allocBindColumns(7));
        visCols << gui::Header(0, "ID",           "ID",           50,  td::HAlignment::Center)
                << gui::Header(1, "Čvor",         "Čvor",         70,  td::HAlignment::Center)
                << gui::Header(2, "Tip čvora",    "Tip čvora",    80,  td::HAlignment::Center)
                << gui::Header(3, "Tip mjerenja", "Tip mjerenja", 100, td::HAlignment::Center)
                << gui::Header(4, "Do čvora",     "Do čvora",     80,  td::HAlignment::Center)
                << gui::Header(5, "Vrijednost",   "Vrijednost",   110, td::HAlignment::Center)
                << gui::Header(6, "Sigma",        "Sigma",        90,  td::HAlignment::Center);

        _table.init(_pDS);
        _table.setColumnNumericFormat(5, td::FormatFloat::Decimal, 6);
        _table.setColumnNumericFormat(6, td::FormatFloat::Decimal, 4);
        return true;
    }
    bool deleteSelectedRow()
    {
        int selRow = _table.getFirstSelectedRow();
        if (selRow < 0)
        {
            showAlert(td::String("Nema odabira"), td::String("Odaberite mjerenje koje zelite obrisati."));
            return false;
        }

        auto& row = _pDS->getRow(selRow);
        int measId = row[0].i4Val();  

        dp::IStatementPtr pDel(_db->createStatement(
            "DELETE FROM Measurements WHERE id = ?"));
        dp::Params par(pDel->allocParams());
        td::Variant idVar(measId);
        par << idVar;

        if (!pDel->execute())
        {
            showAlert(td::String("Greska"), td::String("Brisanje nije uspjelo!"));
            return false;
        }

        _table.reload();
        return true;
    }
    int getNextMeasIndex(int nodeId, const char* measType)
    {
        td::String sql;
        sql.format("SELECT meas_index FROM Measurements "
                   "WHERE node_id = %d AND meas_type = '%s' ORDER BY meas_index",
                   nodeId, measType);
        auto pDS = _db->createDataSet(sql.c_str(), dp::IDataSet::Execution::EX_MULT);
        dp::DSColumns cols(pDS->allocBindColumns(1));
        cols << "meas_index" << td::int4;
        if (!pDS->execute()) return 1;
        int expected = 1;
        for (size_t i = 0; i < pDS->getNumberOfRows(); ++i)
        {
            if (pDS->getRow(i)[0].i4Val() != expected) return expected;
            ++expected;
        }
        return expected;
    }

    bool addMeasurement()
    {
        int nodeIdx = _inputView._cbNodeID.getSelectedIndex();
        if (nodeIdx < 0) { _log.appendString("Greška: odaberite čvor!\n"); return false; }
        int nodeId = nodeIdx + 1;

        int typeIdx = _inputView._cbMeasType.getSelectedIndex();
        if (typeIdx < 0) { _log.appendString("Greška: odaberite tip mjerenja!\n"); return false; }

        static const char* measTypes[]  = {"P_INJ", "Q_INJ", "V_MAG", "P_FLOW", "Q_FLOW"};
        static const char* typeLabels[] = {"P (inj.)", "Q (inj.)", "|V|", "P (tok)", "Q (tok)"};
        const char* measType = measTypes[typeIdx];
        bool isFlow = (typeIdx == 3 || typeIdx == 4);
        int toNodeId = nodeId;

        if (isFlow)
        {
            int toIdx = _inputView._cbToNode.getSelectedIndex();
            if (toIdx < 0)
            { _log.appendString("Greška: odaberite odredišni čvor ('Do čvora')!\n"); return false; }
            toNodeId = toIdx + 1;
            if (toNodeId == nodeId)
            { _log.appendString("Greška: 'Čvor' i 'Do čvora' ne mogu biti isti!\n"); return false; }
        }
        int measIndex = getNextMeasIndex(nodeId, measType);

        td::Variant val; _inputView._valueEdit.getValue(val);
        td::Variant sg;  _inputView._sigmaEdit.getValue(sg);

        dp::IStatementPtr pIns(_db->createStatement(
            "INSERT INTO Measurements (node_id, meas_type, meas_index, to_node, value, sigma) "
            "VALUES (?, ?, ?, ?, ?, ?)"));
        dp::Params par(pIns->allocParams());
        td::Variant nid(nodeId);
        td::Variant mt(td::string8, td::nch, 12);
        td::String  mtStr(measType);
        mt.setValue(mtStr);
        td::Variant idxVar(measIndex);
        td::Variant toNodeVar(toNodeId);
        par << nid << mt << idxVar << toNodeVar << val << sg;

        if (!pIns->execute())
        {
            _log.appendString("Greška pri unosu mjerenja!\n");
            return false;
        }

        td::String msg;
        if (isFlow)
            msg.format("Čvor %d -> %d: dodano mjerenje %s\n", nodeId, toNodeId, typeLabels[typeIdx]);
        else
            msg.format("Čvor %d: dodano mjerenje %s (#%d)\n", nodeId, typeLabels[typeIdx], measIndex);
        _log.appendString(msg);

        _inputView._valueEdit.toZero();
        _table.reload();
        return true;
    }

    bool onClick(gui::Button* pBtn) override
    {
        if (pBtn == &_inputView._btnAddMeas) { addMeasurement();    return true; }
        if (pBtn == &_btnDelete)             { deleteSelectedRow(); return true; }
        if (pBtn == &_btnReload)             { _table.reload();     return true; }
        return false;
    }
    bool onChangedSelection(gui::ComboBox* pCmb) override
    {
        if (pCmb == &_inputView._cbMeasType)
        {
            int typeIdx = _inputView._cbMeasType.getSelectedIndex();
            bool isFlow = (typeIdx == 3 || typeIdx == 4);
            _inputView._cbToNode.enable(isFlow);
            return true;
        }
        return false;
    }
};