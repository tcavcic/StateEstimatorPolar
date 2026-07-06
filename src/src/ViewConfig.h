#pragma once

// =============================================================================
//  ViewConfig.h
// =============================================================================

#include <cmath>
#include <complex>
namespace { constexpr double MY_PI = 3.14159265358979323846; }
#include <gui/View.h>
#include <gui/Label.h>
#include <gui/NumericEdit.h>
#include <gui/Button.h>
#include <gui/CheckBox.h>
#include <gui/TableEdit.h>
#include <gui/GridLayout.h>
#include <gui/GridComposer.h>
#include <gui/ComboBox.h>
#include <dp/IDatabase.h>
#include <dp/IDataSet.h>
#include <fo/FileOperations.h>
#include <functional>

class ConnectionView : public gui::View
{
public:
    gui::Label    _lblFrom;
    gui::ComboBox _cbFrom;
    gui::Label    _lblTo;
    gui::ComboBox _cbTo;
    gui::CheckBox _chkPolar;
    gui::CheckBox _chkSymmetric;
    gui::GridLayout _glConn;

    ConnectionView()
    : _lblFrom("Od cvora:")
    , _lblTo("Do cvora:")
    , _chkPolar("Polarni oblik (|Y|, \xCF\x86)")
    , _chkSymmetric("Simetricna Y (Zs, Ysh)")
    , _glConn(6, 1)
    {
        _chkPolar.setChecked(true);
        _glConn.setSpaceBetweenCells(1, 1);
        _glConn.setMargins(2, 2);

        gui::GridComposer gc(_glConn);
        gc.appendRow(_lblFrom);
        gc.appendRow(_cbFrom);
        gc.appendRow(_lblTo);
        gc.appendRow(_cbTo);
        gc.appendRow(_chkPolar);
        gc.appendRow(_chkSymmetric);
        setLayout(&_glConn);
    }
};

class MatrixView : public gui::View
{
public:
    gui::Label       _lblRow1y11;
    gui::Label       _lblRow1y12;
    gui::Label       _lblRow2y21;
    gui::Label       _lblRow2y22;
    gui::Label       _lblV1y11;  gui::NumericEdit _v1y11;
    gui::Label       _lblV2y11;  gui::NumericEdit _v2y11;
    gui::Label       _lblV1y12;  gui::NumericEdit _v1y12;
    gui::Label       _lblV2y12;  gui::NumericEdit _v2y12;
    gui::Label       _lblV1y21;  gui::NumericEdit _v1y21;
    gui::Label       _lblV2y21;  gui::NumericEdit _v2y21;
    gui::Label       _lblV1y22;  gui::NumericEdit _v1y22;
    gui::Label       _lblV2y22;  gui::NumericEdit _v2y22;
    gui::Label       _lblY0;
    gui::Label       _lblV1y0;   gui::NumericEdit _v1y0;
    gui::Label       _lblV2y0;   gui::NumericEdit _v2y0;

    gui::Label       _lblZs;
    gui::Label       _lblV1zs;   gui::NumericEdit _v1zs;
    gui::Label       _lblV2zs;   gui::NumericEdit _v2zs;
    gui::Label       _lblYsh;
    gui::Label       _lblV1ysh;  gui::NumericEdit _v1ysh;
    gui::Label       _lblV2ysh;  gui::NumericEdit _v2ysh;

    gui::GridLayout  _glMatrix;

    MatrixView()
    : _lblRow1y11("*****  y11  *****")
    , _lblRow1y12("*****  y12  *****")
    , _lblRow2y21("*****  y21  *****")
    , _lblRow2y22("*****  y22  *****")
    , _lblV1y11("|Y|:"),    _v1y11(td::real8)
    , _lblV2y11("\xCF\x86 (\xC2\xB0):"), _v2y11(td::real8)
    , _lblV1y12("|Y|:"),    _v1y12(td::real8)
    , _lblV2y12("\xCF\x86 (\xC2\xB0):"), _v2y12(td::real8)
    , _lblV1y21("|Y|:"),    _v1y21(td::real8)
    , _lblV2y21("\xCF\x86 (\xC2\xB0):"), _v2y21(td::real8)
    , _lblV1y22("|Y|:"),    _v1y22(td::real8)
    , _lblV2y22("\xCF\x86 (\xC2\xB0):"), _v2y22(td::real8)
    , _lblY0("y0 (shunt):")
    , _lblV1y0("|Y|:"),     _v1y0(td::real8)
    , _lblV2y0("\xCF\x86 (\xC2\xB0):"), _v2y0(td::real8)
    , _lblZs("Zs (serijska impedansa):")
    , _lblV1zs("|Z|:"),     _v1zs(td::real8)
    , _lblV2zs("\xCF\x86 (\xC2\xB0):"), _v2zs(td::real8)
    , _lblYsh("Ysh (poprecna admitansa):")
    , _lblV1ysh("|Y|:"),    _v1ysh(td::real8)
    , _lblV2ysh("\xCF\x86 (\xC2\xB0):"), _v2ysh(td::real8)
    , _glMatrix(16, 4)
    {
        _glMatrix.setSpaceBetweenCells(1, 1);
        _glMatrix.setMargins(2, 2);

        gui::GridComposer gc(_glMatrix);
        gc.appendRow(_lblRow1y11, 2);
        gc.appendCol(_lblRow1y12, 2);
        gc.appendRow(_lblV1y11);
        gc.appendCol(_v1y11);
        gc.appendCol(_lblV1y12);
        gc.appendCol(_v1y12);
        gc.appendRow(_lblV2y11);
        gc.appendCol(_v2y11);
        gc.appendCol(_lblV2y12);
        gc.appendCol(_v2y12);
        gc.appendRow(_lblRow2y21, 2);
        gc.appendCol(_lblRow2y22, 2);
        gc.appendRow(_lblV1y21);
        gc.appendCol(_v1y21);
        gc.appendCol(_lblV1y22);
        gc.appendCol(_v1y22);
        gc.appendRow(_lblV2y21);
        gc.appendCol(_v2y21);
        gc.appendCol(_lblV2y22);
        gc.appendCol(_v2y22);
        gc.appendRow(_lblY0, 4);
        gc.appendRow(_lblV1y0);
        gc.appendCol(_v1y0);
        gc.appendRow(_lblV2y0);
        gc.appendCol(_v2y0);

        gc.appendRow(_lblZs, 4);
        gc.appendRow(_lblV1zs);
        gc.appendCol(_v1zs);
        gc.appendRow(_lblV2zs);
        gc.appendCol(_v2zs);

        gc.appendRow(_lblYsh, 4);
        gc.appendRow(_lblV1ysh);
        gc.appendCol(_v1ysh);
        gc.appendRow(_lblV2ysh);
        gc.appendCol(_v2ysh);

        setLayout(&_glMatrix);
    }
};

class NodesView : public gui::View
{
public:
    gui::Label       _lblNumNodes;
    gui::NumericEdit _numNodes;
    gui::Button      _btnApplyNodes;
    gui::Label       _lblNodes;
    gui::TableEdit   _tableNodes;
    gui::Label       _lblNodeType;
    gui::ComboBox    _cbNodeType;
    gui::GridLayout  _glNodes;

    NodesView()
    : _lblNumNodes("Broj cvorova:")
    , _numNodes(td::int4)
    , _btnApplyNodes("Primijeni")
    , _lblNodes("Cvorovi:")
    , _lblNodeType("Tip cvora:")
    , _glNodes(7, 2)
    {
        _cbNodeType.addItem("PQ");
        _cbNodeType.addItem("PV");
        _cbNodeType.addItem("SLACK");
        _cbNodeType.selectIndex(0);

        _glNodes.setSpaceBetweenCells(1, 1);
        _glNodes.setMargins(2, 2);

        gui::GridComposer gc(_glNodes);
        gc.appendRow(_lblNumNodes);
        gc.appendCol(_numNodes);
        gc.appendRow(_btnApplyNodes, 2);
        gc.appendRow(_lblNodes, 2);
        gc.appendRow(_tableNodes, 2);
        gc.appendRow(_lblNodeType);
        gc.appendCol(_cbNodeType);
        setLayout(&_glNodes);
    }
};

class InputView : public gui::View
{
public:
    ConnectionView  _connView;
    MatrixView      _matrixView;
    gui::GridLayout _glInput;

    InputView()
    : _glInput(2, 1)
    {
        _glInput.setSpaceBetweenCells(1, 1);
        _glInput.setMargins(0, 0);

        gui::GridComposer gc(_glInput);
        gc.appendRow(_connView);
        gc.appendRow(_matrixView);
        setLayout(&_glInput);
    }
};

class ConfigMainView : public gui::View
{
public:
    InputView       _inputView;
    NodesView       _nodesView;
    gui::GridLayout _glMain;

    ConfigMainView()
    : _glMain(1, 2)
    {
        _glMain.setSpaceBetweenCells(1, 1);
        _glMain.setMargins(0, 0);

        gui::GridComposer gc(_glMain);
        gc.appendRow(_inputView);
        gc.appendCol(_nodesView);
        setLayout(&_glMain);
    }
};

#define _conn      _configMainView._inputView._connView
#define _matrix    _configMainView._inputView._matrixView
#define _nodes     _configMainView._nodesView

class ViewConfig : public gui::View
{
public:
    void setConfigChangedCallback(std::function<void()> cb)
    {
        _onConfigChanged = cb;
    }

protected:
    ConfigMainView    _configMainView;

    gui::Button      _btnAddBranch;
    gui::Label       _lblBranches;
    gui::TableEdit   _tableBranches;
    gui::Button      _btnDeleteBranch;

    gui::GridLayout  _gl;

    dp::IDatabasePtr _db;
    dp::IDataSetPtr  _pDSNodes;
    dp::IDataSetPtr  _pDSBranches;

    std::function<void()> _onConfigChanged;
    bool _blockTypeCallback = false;
    bool _isPolar = true;
    bool _isSelfLoop = false;
    bool _isSymmetric = false;

    static std::complex<double> polarToComplex(double r, double phiDeg)
    {
        double rad = phiDeg * MY_PI / 180.0;
        return std::complex<double>(r * std::cos(rad), r * std::sin(rad));
    }

    static void complexToPolar(const std::complex<double>& c, double& r, double& phiDeg)
    {
        r = std::abs(c);
        phiDeg = std::arg(c) * 180.0 / MY_PI;
    }

    std::complex<double> fieldsToComplex(gui::NumericEdit& ne1, gui::NumericEdit& ne2)
    {
        double v1 = fieldVal(ne1), v2 = fieldVal(ne2);
        return _isPolar ? polarToComplex(v1, v2) : std::complex<double>(v1, v2);
    }

    void setComplexToFields(gui::NumericEdit& ne1, gui::NumericEdit& ne2,
                             const std::complex<double>& c)
    {
        if (_isPolar)
        {
            double r, phi;
            complexToPolar(c, r, phi);
            setField(ne1, r); setField(ne2, phi);
        }
        else
        {
            setField(ne1, c.real()); setField(ne2, c.imag());
        }
    }

    void setMatrixFieldsFromPolar(double r11, double phi11,
                                   double r12, double phi12,
                                   double r21, double phi21,
                                   double r22, double phi22,
                                   double r0,  double phi0)
    {
        if (_isPolar)
        {
            setField(_matrix._v1y11, r11);  setField(_matrix._v2y11, phi11);
            setField(_matrix._v1y12, r12);  setField(_matrix._v2y12, phi12);
            setField(_matrix._v1y21, r21);  setField(_matrix._v2y21, phi21);
            setField(_matrix._v1y22, r22);  setField(_matrix._v2y22, phi22);
            setField(_matrix._v1y0,  r0);   setField(_matrix._v2y0,  phi0);
        }
        else
        {
            auto toRect = [](double r, double phi_deg, double& re, double& im)
            {
                double rad = phi_deg * MY_PI / 180.0;
                re = r * std::cos(rad);
                im = r * std::sin(rad);
            };
            double re, im;
            toRect(r11, phi11, re, im); setField(_matrix._v1y11, re); setField(_matrix._v2y11, im);
            toRect(r12, phi12, re, im); setField(_matrix._v1y12, re); setField(_matrix._v2y12, im);
            toRect(r21, phi21, re, im); setField(_matrix._v1y21, re); setField(_matrix._v2y21, im);
            toRect(r22, phi22, re, im); setField(_matrix._v1y22, re); setField(_matrix._v2y22, im);
            toRect(r0,  phi0,  re, im); setField(_matrix._v1y0,  re); setField(_matrix._v2y0,  im);
        }
    }

    void updateVisibility(bool selfLoop)
    {
        _isSelfLoop = selfLoop;
        bool hideMatrix = selfLoop || _isSymmetric;
        bool hideY0     = !selfLoop;
        bool hideSym    = selfLoop || !_isSymmetric;

        _matrix._lblRow1y11.hide(hideMatrix, false);
        _matrix._lblRow1y12.hide(hideMatrix, false);
        _matrix._lblV1y11.hide(hideMatrix, false); _matrix._v1y11.hide(hideMatrix, false);
        _matrix._lblV2y11.hide(hideMatrix, false); _matrix._v2y11.hide(hideMatrix, false);
        _matrix._lblV1y12.hide(hideMatrix, false); _matrix._v1y12.hide(hideMatrix, false);
        _matrix._lblV2y12.hide(hideMatrix, false); _matrix._v2y12.hide(hideMatrix, false);
        _matrix._lblRow2y21.hide(hideMatrix, false);
        _matrix._lblRow2y22.hide(hideMatrix, false);
        _matrix._lblV1y21.hide(hideMatrix, false); _matrix._v1y21.hide(hideMatrix, false);
        _matrix._lblV2y21.hide(hideMatrix, false); _matrix._v2y21.hide(hideMatrix, false);
        _matrix._lblV1y22.hide(hideMatrix, false); _matrix._v1y22.hide(hideMatrix, false);
        _matrix._lblV2y22.hide(hideMatrix, false); _matrix._v2y22.hide(hideMatrix, false);

        _matrix._lblY0.hide(hideY0, false);
        _matrix._lblV1y0.hide(hideY0, false); _matrix._v1y0.hide(hideY0, false);
        _matrix._lblV2y0.hide(hideY0, false); _matrix._v2y0.hide(hideY0, true);

        _matrix._lblZs.hide(hideSym, false);
        _matrix._lblV1zs.hide(hideSym, false); _matrix._v1zs.hide(hideSym, false);
        _matrix._lblV2zs.hide(hideSym, false); _matrix._v2zs.hide(hideSym, false);
        _matrix._lblYsh.hide(hideSym, false);
        _matrix._lblV1ysh.hide(hideSym, false); _matrix._v1ysh.hide(hideSym, false);
        _matrix._lblV2ysh.hide(hideSym, false); _matrix._v2ysh.hide(hideSym, true);

        _conn._chkSymmetric.hide(selfLoop, false);
    }

public:
    ViewConfig()
    : _btnAddBranch("Dodaj vezu")
    , _lblBranches("Veze:")
    , _btnDeleteBranch("Obrisi vezu")
    , _gl(5, 1)
    , _db(dp::create(dp::IDatabase::ConnType::CT_SQLITE,
                     dp::IDatabase::ServerType::SER_SQLITE3))
    {
        _gl.setSpaceBetweenCells(2, 1);
        _gl.setMargins(3, 3);

        gui::GridComposer gc(_gl);
        gc.appendRow(_configMainView);
        gc.appendRow(_btnAddBranch);
        gc.appendRow(_lblBranches);
        gc.appendRow(_tableBranches);
        gc.appendRow(_btnDeleteBranch);
        setLayout(&_gl);

        _nodes._btnApplyNodes.forwardMessagesTo(this);
        _btnAddBranch.forwardMessagesTo(this);
        _btnDeleteBranch.forwardMessagesTo(this);
        _conn._cbFrom.forwardMessagesTo(this);
        _conn._cbTo.forwardMessagesTo(this);

        _nodes._tableNodes.onChangedSelection([this](td::INT4 rowNo)
        {
            if (rowNo < 0 || !_pDSNodes) return;
            _blockTypeCallback = true;
            auto& row = _pDSNodes->getRow(rowNo);
            td::String nodeType = row[1].strVal();
            if (nodeType == td::String("PV"))         _nodes._cbNodeType.selectIndex(1);
            else if (nodeType == td::String("SLACK"))  _nodes._cbNodeType.selectIndex(2);
            else                                       _nodes._cbNodeType.selectIndex(0);
            _blockTypeCallback = false;
        });

        _nodes._cbNodeType.onChangedSelection([this]()
        {
            if (_blockTypeCallback) return;
            setNodeType();
        });

        _tableBranches.onChangedSelection([this](td::INT4 rowNo)
        {
            if (rowNo < 0 || !_pDSBranches) return;
            auto& row = _pDSBranches->getRow(rowNo);
            double r11  = row[3].r8Val(),  phi11 = row[4].r8Val();
            double r12  = row[5].r8Val(),  phi12 = row[6].r8Val();
            double r21  = row[7].r8Val(),  phi21 = row[8].r8Val();
            double r22  = row[9].r8Val(),  phi22 = row[10].r8Val();
            double r0   = row[11].r8Val(), phi0  = row[12].r8Val();
            bool selfLoop = (row[1].i4Val() == row[2].i4Val());

            setMatrixFieldsFromPolar(r11, phi11, r12, phi12,
                                     r21, phi21, r22, phi22, r0, phi0);

            if (!selfLoop)
            {
                std::complex<double> y11c = polarToComplex(r11, phi11);
                std::complex<double> y12c = polarToComplex(r12, phi12);
                if (std::abs(y12c) > 1e-12)
                {
                    std::complex<double> zsC  = -1.0 / y12c;
                    std::complex<double> yshC = 2.0 * (y11c + y12c);
                    setComplexToFields(_matrix._v1zs,  _matrix._v2zs,  zsC);
                    setComplexToFields(_matrix._v1ysh, _matrix._v2ysh, yshC);
                }
            }

            updateVisibility(selfLoop);
        });

        _conn._chkPolar.onClick([this]()
        {
            bool newPolar = _conn._chkPolar.isChecked();
            if (newPolar == _isPolar) return;
            if (newPolar)
                convertFieldsToDisplay_RectToPolar();
            else
                convertFieldsToDisplay_PolarToRect();
            _isPolar = newPolar;
            updateFormatLabels();
        });

        _conn._chkSymmetric.onClick([this]()
        {
            _isSymmetric = _conn._chkSymmetric.isChecked();
            updateVisibility(_isSelfLoop);
        });

        initDB();
        loadNodes();
        loadBranches();
        updateVisibility(false);
        updateFormatLabels();
    }

    int getNumNodes()
    {
        return (int)_pDSNodes->getNumberOfRows();
    }

    td::String getNodeType(int nodeId)
    {
        size_t nRows = _pDSNodes->getNumberOfRows();
        for (size_t i = 0; i < nRows; ++i)
        {
            auto& row = _pDSNodes->getRow(i);
            if (row[0].i4Val() == nodeId)
                return row[1].strVal();
        }
        return td::String("PQ");
    }

    void populateNodeCombo(gui::ComboBox& cb)
    {
        cb.clean();
        size_t nRows = _pDSNodes->getNumberOfRows();
        for (size_t i = 0; i < nRows; ++i)
        {
            auto& row = _pDSNodes->getRow(i);
            td::String item;
            item.format("Cvor %d (%s)", row[0].i4Val(), row[1].strVal().c_str());
            cb.addItem(item);
        }
    }

protected:

    double fieldVal(gui::NumericEdit& ne)
    {
        td::Variant v;
        ne.getValue(v);
        return v.r8Val();
    }

    void setField(gui::NumericEdit& ne, double val)
    {
        td::Variant v(val);
        ne.setValue(v);
    }

    void getFieldsAsPolar(double& r11, double& phi11,
                          double& r12, double& phi12,
                          double& r21, double& phi21,
                          double& r22, double& phi22,
                          double& r0,  double& phi0)
    {
        if (_isPolar)
        {
            r11  = fieldVal(_matrix._v1y11); phi11 = fieldVal(_matrix._v2y11);
            r12  = fieldVal(_matrix._v1y12); phi12 = fieldVal(_matrix._v2y12);
            r21  = fieldVal(_matrix._v1y21); phi21 = fieldVal(_matrix._v2y21);
            r22  = fieldVal(_matrix._v1y22); phi22 = fieldVal(_matrix._v2y22);
            r0   = fieldVal(_matrix._v1y0);  phi0  = fieldVal(_matrix._v2y0);
        }
        else
        {
            auto toPolar = [](double re, double im, double& r, double& phi_deg)
            {
                r       = std::sqrt(re*re + im*im);
                phi_deg = std::atan2(im, re) * 180.0 / MY_PI;
            };
            toPolar(fieldVal(_matrix._v1y11), fieldVal(_matrix._v2y11), r11, phi11);
            toPolar(fieldVal(_matrix._v1y12), fieldVal(_matrix._v2y12), r12, phi12);
            toPolar(fieldVal(_matrix._v1y21), fieldVal(_matrix._v2y21), r21, phi21);
            toPolar(fieldVal(_matrix._v1y22), fieldVal(_matrix._v2y22), r22, phi22);
            toPolar(fieldVal(_matrix._v1y0),  fieldVal(_matrix._v2y0),  r0,  phi0);
        }
    }

    void convertFieldsToDisplay_RectToPolar()
    {
        auto conv = [&](gui::NumericEdit& ne1, gui::NumericEdit& ne2)
        {
            double re = fieldVal(ne1), im = fieldVal(ne2);
            double mag = std::sqrt(re*re + im*im);
            double ang = std::atan2(im, re) * 180.0 / MY_PI;
            setField(ne1, mag); setField(ne2, ang);
        };
        conv(_matrix._v1y11, _matrix._v2y11); conv(_matrix._v1y12, _matrix._v2y12);
        conv(_matrix._v1y21, _matrix._v2y21); conv(_matrix._v1y22, _matrix._v2y22);
        conv(_matrix._v1y0,  _matrix._v2y0);
        conv(_matrix._v1zs,  _matrix._v2zs);
        conv(_matrix._v1ysh, _matrix._v2ysh);
    }

    void convertFieldsToDisplay_PolarToRect()
    {
        auto conv = [&](gui::NumericEdit& ne1, gui::NumericEdit& ne2)
        {
            double mag = fieldVal(ne1), ang = fieldVal(ne2);
            double rad = ang * MY_PI / 180.0;
            setField(ne1, mag * std::cos(rad));
            setField(ne2, mag * std::sin(rad));
        };
        conv(_matrix._v1y11, _matrix._v2y11); conv(_matrix._v1y12, _matrix._v2y12);
        conv(_matrix._v1y21, _matrix._v2y21); conv(_matrix._v1y22, _matrix._v2y22);
        conv(_matrix._v1y0,  _matrix._v2y0);
        conv(_matrix._v1zs,  _matrix._v2zs);
        conv(_matrix._v1ysh, _matrix._v2ysh);
    }

    void updateFormatLabels()
    {
        if (_isPolar)
        {
            _matrix._lblV1y11.setTitle("|Y|:"); _matrix._lblV2y11.setTitle("\xCF\x86 (\xC2\xB0):");
            _matrix._lblV1y12.setTitle("|Y|:"); _matrix._lblV2y12.setTitle("\xCF\x86 (\xC2\xB0):");
            _matrix._lblV1y21.setTitle("|Y|:"); _matrix._lblV2y21.setTitle("\xCF\x86 (\xC2\xB0):");
            _matrix._lblV1y22.setTitle("|Y|:"); _matrix._lblV2y22.setTitle("\xCF\x86 (\xC2\xB0):");
            _matrix._lblV1y0.setTitle("|Y|:");  _matrix._lblV2y0.setTitle("\xCF\x86 (\xC2\xB0):");
            _matrix._lblV1zs.setTitle("|Z|:");  _matrix._lblV2zs.setTitle("\xCF\x86 (\xC2\xB0):");
            _matrix._lblV1ysh.setTitle("|Y|:"); _matrix._lblV2ysh.setTitle("\xCF\x86 (\xC2\xB0):");
        }
        else
        {
            _matrix._lblV1y11.setTitle("Re:"); _matrix._lblV2y11.setTitle("Im:");
            _matrix._lblV1y12.setTitle("Re:"); _matrix._lblV2y12.setTitle("Im:");
            _matrix._lblV1y21.setTitle("Re:"); _matrix._lblV2y21.setTitle("Im:");
            _matrix._lblV1y22.setTitle("Re:"); _matrix._lblV2y22.setTitle("Im:");
            _matrix._lblV1y0.setTitle("Re:");  _matrix._lblV2y0.setTitle("Im:");
            _matrix._lblV1zs.setTitle("Re:");  _matrix._lblV2zs.setTitle("Im:");
            _matrix._lblV1ysh.setTitle("Re:"); _matrix._lblV2ysh.setTitle("Im:");
        }
    }

    void initDB()
    {
        td::String dbFileName = gui::getResFileName(":Baza.db");
        if (!_db->connect(dbFileName))
        {
            showAlert("DB error", "Ne mogu otvoriti Baza.db");
            return;
        }
        dp::IStatementPtr s1(_db->createStatement(
            "CREATE TABLE IF NOT EXISTS Nodes ("
            "node_id INTEGER PRIMARY KEY, "
            "node_type TEXT NOT NULL DEFAULT 'PQ')"));
        s1->execute();

        dp::IStatementPtr s2(_db->createStatement(
            "CREATE TABLE IF NOT EXISTS Branches ("
            "id        INTEGER PRIMARY KEY AUTOINCREMENT, "
            "from_node INTEGER NOT NULL, "
            "to_node   INTEGER NOT NULL, "
            "y11_r     REAL NOT NULL DEFAULT 0.0, "
            "y11_phi   REAL NOT NULL DEFAULT 0.0, "
            "y12_r     REAL NOT NULL DEFAULT 0.0, "
            "y12_phi   REAL NOT NULL DEFAULT 0.0, "
            "y21_r     REAL NOT NULL DEFAULT 0.0, "
            "y21_phi   REAL NOT NULL DEFAULT 0.0, "
            "y22_r     REAL NOT NULL DEFAULT 0.0, "
            "y22_phi   REAL NOT NULL DEFAULT 0.0, "
            "y0_r      REAL NOT NULL DEFAULT 0.0, "
            "y0_phi    REAL NOT NULL DEFAULT 0.0, "
            "FOREIGN KEY(from_node) REFERENCES Nodes(node_id), "
            "FOREIGN KEY(to_node)   REFERENCES Nodes(node_id))"));
        s2->execute();

        dp::IStatementPtr s3(_db->createStatement(
            "CREATE TABLE IF NOT EXISTS Measurements ("
            "id         INTEGER PRIMARY KEY AUTOINCREMENT, "
            "node_id    INTEGER NOT NULL, "
            "meas_type  TEXT NOT NULL, "
            "meas_index INTEGER NOT NULL, "
            "to_node    INTEGER, "
            "value      REAL NOT NULL, "
            "sigma      REAL NOT NULL DEFAULT 0.01, "
            "FOREIGN KEY(node_id) REFERENCES Nodes(node_id), "
            "FOREIGN KEY(to_node) REFERENCES Nodes(node_id))"));
        s3->execute();
    }

    void loadNodes()
    {
        _pDSNodes = _db->createDataSet(
            "SELECT node_id, node_type FROM Nodes ORDER BY node_id",
            dp::IDataSet::Execution::EX_MULT);

        dp::DSColumns cols(_pDSNodes->allocBindColumns(2));
        cols << "node_id"   << td::int4
             << "node_type" << td::string8;
        _pDSNodes->execute();

        gui::Columns visCols(_nodes._tableNodes.allocBindColumns(2));
        visCols << gui::Header(0, "ID cvora",  "ID cvora",  150, td::HAlignment::Center)
                << gui::Header(1, "Tip cvora", "Tip cvora", 150, td::HAlignment::Center);
        _nodes._tableNodes.init(_pDSNodes);

        int n = (int)_pDSNodes->getNumberOfRows();
        td::Variant v(n);
        _nodes._numNodes.setValue(v);

        populateNodeCombo(_conn._cbFrom);
        populateNodeCombo(_conn._cbTo);
    }

    void loadBranches()
    {
        _pDSBranches = _db->createDataSet(
            "SELECT id, from_node, to_node, "
            "y11_r, y11_phi, y12_r, y12_phi, "
            "y21_r, y21_phi, y22_r, y22_phi, "
            "y0_r, y0_phi "
            "FROM Branches ORDER BY id",
            dp::IDataSet::Execution::EX_MULT);

        dp::DSColumns cols(_pDSBranches->allocBindColumns(13));
        cols << "id"        << td::int4
             << "from_node" << td::int4
             << "to_node"   << td::int4
             << "y11_r"     << td::real8
             << "y11_phi"   << td::real8
             << "y12_r"     << td::real8
             << "y12_phi"   << td::real8
             << "y21_r"     << td::real8
             << "y21_phi"   << td::real8
             << "y22_r"     << td::real8
             << "y22_phi"   << td::real8
             << "y0_r"      << td::real8
             << "y0_phi"    << td::real8;
        _pDSBranches->execute();

        gui::Columns visCols(_tableBranches.allocBindColumns(12));
        visCols << gui::Header(1,  "Od",           "Od",   35, td::HAlignment::Center)
                << gui::Header(2,  "Do",           "Do",   35, td::HAlignment::Center)
                << gui::Header(3,  "y11 R",        "y11r", 60, td::HAlignment::Center)
                << gui::Header(4,  "y11 \xCF\x86", "y11p", 60, td::HAlignment::Center)
                << gui::Header(5,  "y12 R",        "y12r", 60, td::HAlignment::Center)
                << gui::Header(6,  "y12 \xCF\x86", "y12p", 60, td::HAlignment::Center)
                << gui::Header(7,  "y21 R",        "y21r", 60, td::HAlignment::Center)
                << gui::Header(8,  "y21 \xCF\x86", "y21p", 60, td::HAlignment::Center)
                << gui::Header(9,  "y22 R",        "y22r", 60, td::HAlignment::Center)
                << gui::Header(10, "y22 \xCF\x86", "y22p", 60, td::HAlignment::Center)
                << gui::Header(11, "y0 R",         "y0r",  60, td::HAlignment::Center)
                << gui::Header(12, "y0 \xCF\x86",  "y0p",  60, td::HAlignment::Center);
        _tableBranches.init(_pDSBranches);
        for (int c = 3; c <= 12; ++c)
            _tableBranches.setColumnNumericFormat(c, td::FormatFloat::Decimal, 4);
    }

    void refreshNodes()
    {
        int selRow = _nodes._tableNodes.getFirstSelectedRow();
        _blockTypeCallback = true;
        _nodes._tableNodes.reload();
        _blockTypeCallback = false;

        int n = (int)_nodes._tableNodes.getNumberOfRows();
        td::Variant v(n);
        _nodes._numNodes.setValue(v);

        populateNodeCombo(_conn._cbFrom);
        populateNodeCombo(_conn._cbTo);

        if (selRow >= 0 && selRow < n)
            _nodes._tableNodes.selectRow(selRow, false, true);
    }

    void refreshBranches()
    {
        _tableBranches.reload();
    }

    void applyNumNodes()
    {
        td::Variant v;
        _nodes._numNodes.getValue(v);
        int newN = v.i4Val();
        if (newN <= 0)
        {
            showAlert(td::String("Greska"), td::String("Broj cvorova mora biti veci od 0."));
            return;
        }

        int currentN = (int)_nodes._tableNodes.getNumberOfRows();

        if (newN > currentN)
        {
            dp::IStatementPtr pIns(_db->createStatement(
                "INSERT OR IGNORE INTO Nodes (node_id, node_type) VALUES (?, 'PQ')"));
            for (int i = currentN + 1; i <= newN; ++i)
            {
                dp::Params par(pIns->allocParams());
                td::Variant id(i);
                par << id;
                pIns->execute();
            }
        }
        else if (newN < currentN)
        {
            dp::IStatementPtr pDelB(_db->createStatement(
                "DELETE FROM Branches WHERE from_node > ? OR to_node > ?"));
            dp::Params parB(pDelB->allocParams());
            td::Variant nv(newN);
            parB << nv << nv;
            pDelB->execute();

            dp::IStatementPtr pDelN(_db->createStatement(
                "DELETE FROM Nodes WHERE node_id > ?"));
            dp::Params parN(pDelN->allocParams());
            td::Variant nv2(newN);
            parN << nv2;
            pDelN->execute();
        }

        refreshNodes();
        refreshBranches();

        if (_onConfigChanged)
            _onConfigChanged();
    }

    void setNodeType()
    {
        int selRow = _nodes._tableNodes.getFirstSelectedRow();
        if (selRow < 0) return;

        auto& row = _pDSNodes->getRow(selRow);
        int nodeId = row[0].i4Val();

        int typeIdx = _nodes._cbNodeType.getSelectedIndex();
        const char* typeNames[] = {"PQ", "PV", "SLACK"};
        if (typeIdx < 0 || typeIdx > 2) return;

        td::String sqlCheck;
        sqlCheck.format(
            "SELECT COUNT(*) as cnt FROM Measurements WHERE node_id = %d", nodeId);
        auto pDSCheck = _db->createDataSet(sqlCheck.c_str(), dp::IDataSet::Execution::EX_MULT);
        dp::DSColumns checkCols(pDSCheck->allocBindColumns(1));
        checkCols << "cnt" << td::int4;

        if (pDSCheck->execute() && pDSCheck->getNumberOfRows() > 0)
        {
            int cnt = pDSCheck->getRow(0)[0].i4Val();
            if (cnt > 0)
            {
                td::String msg;
                msg.format(
                    "Cvor %d ima %d mjerenja u bazi.\n"
                    "Obrisite sva mjerenja za ovaj cvor u tabu "
                    "\"Pregled mjerenja\" pa pokusajte ponovo.", nodeId, cnt);
                showAlert(td::String("Nije moguce promijeniti tip"), msg);

                _blockTypeCallback = true;
                td::String oldType = row[1].strVal();
                if (oldType == td::String("PV"))         _nodes._cbNodeType.selectIndex(1);
                else if (oldType == td::String("SLACK"))  _nodes._cbNodeType.selectIndex(2);
                else                                      _nodes._cbNodeType.selectIndex(0);
                _blockTypeCallback = false;
                return;
            }
        }

        td::String newType(typeNames[typeIdx]);

        dp::IStatementPtr pUpd(_db->createStatement(
            "UPDATE Nodes SET node_type = ? WHERE node_id = ?"));
        dp::Params par(pUpd->allocParams());
        td::Variant typeVar(td::string8, td::nch, 10);
        typeVar.setValue(newType);
        td::Variant idVar(nodeId);
        par << typeVar << idVar;

        if (!pUpd->execute())
        {
            showAlert(td::String("Greska"), td::String("Ne mogu azurirati tip cvora."));
            return;
        }

        refreshNodes();

        if (_onConfigChanged)
            _onConfigChanged();
    }

    void addBranch()
    {
        int fromIdx = _conn._cbFrom.getSelectedIndex();
        int toIdx   = _conn._cbTo.getSelectedIndex();
        if (fromIdx < 0 || toIdx < 0)
        {
            showAlert(td::String("Greska"), td::String("Odaberite oba cvora."));
            return;
        }

        int fromId   = fromIdx + 1;
        int toId     = toIdx   + 1;
        bool selfLoop = (fromId == toId);

        if (!selfLoop)
        {
            td::String sqlCheck;
            sqlCheck.format(
                "SELECT COUNT(*) as cnt FROM Branches "
                "WHERE (from_node = %d AND to_node = %d) "
                "OR (from_node = %d AND to_node = %d)",
                fromId, toId, toId, fromId);
            auto pDSCheck = _db->createDataSet(sqlCheck.c_str(), dp::IDataSet::Execution::EX_MULT);
            dp::DSColumns checkCols(pDSCheck->allocBindColumns(1));
            checkCols << "cnt" << td::int4;

            if (pDSCheck->execute() && pDSCheck->getNumberOfRows() > 0)
            {
                if (pDSCheck->getRow(0)[0].i4Val() > 0)
                {
                    td::String msg;
                    msg.format("Veza izmedju cvora %d i cvora %d vec postoji.", fromId, toId);
                    showAlert(td::String("Dupla veza"), msg);
                    return;
                }
            }
        }

        double r11=0, phi11=0, r12=0, phi12=0;
        double r21=0, phi21=0, r22=0, phi22=0;
        double r0=0,  phi0=0;

        if (selfLoop)
        {
            r0   = fieldVal(_matrix._v1y0);
            phi0 = fieldVal(_matrix._v2y0);
            if (!_isPolar)
            {
                double re = r0, im = phi0;
                r0   = std::sqrt(re*re + im*im);
                phi0 = std::atan2(im, re) * 180.0 / MY_PI;
            }
        }
        else if (_isSymmetric)
        {
            std::complex<double> zs  = fieldsToComplex(_matrix._v1zs,  _matrix._v2zs);
            std::complex<double> ysh = fieldsToComplex(_matrix._v1ysh, _matrix._v2ysh);

            if (std::abs(zs) < 1e-12)
            {
                showAlert(td::String("Greska"), td::String("Zs ne moze biti 0."));
                return;
            }

            std::complex<double> y11c = 1.0 / zs + ysh / 2.0;
            std::complex<double> y12c = -1.0 / zs;

            complexToPolar(y11c, r11, phi11);
            complexToPolar(y12c, r12, phi12);
            r22 = r11; phi22 = phi11;
            r21 = r12; phi21 = phi12;
        }
        else
            getFieldsAsPolar(r11, phi11, r12, phi12, r21, phi21, r22, phi22, r0, phi0);

        dp::IStatementPtr pIns(_db->createStatement(
            "INSERT INTO Branches "
            "(from_node, to_node, "
            " y11_r, y11_phi, y12_r, y12_phi, "
            " y21_r, y21_phi, y22_r, y22_phi, "
            " y0_r, y0_phi) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));

        dp::Params par(pIns->allocParams());
        td::Variant fv(fromId), tv(toId);
        td::Variant vr11(r11),  vphi11(phi11);
        td::Variant vr12(r12),  vphi12(phi12);
        td::Variant vr21(r21),  vphi21(phi21);
        td::Variant vr22(r22),  vphi22(phi22);
        td::Variant vr0(r0),    vphi0(phi0);
        par << fv    << tv
            << vr11  << vphi11
            << vr12  << vphi12
            << vr21  << vphi21
            << vr22  << vphi22
            << vr0   << vphi0;

        if (!pIns->execute())
        {
            showAlert(td::String("Greska"), td::String("Dodavanje veze nije uspjelo."));
            return;
        }

        _matrix._v1y11.toZero(); _matrix._v2y11.toZero();
        _matrix._v1y12.toZero(); _matrix._v2y12.toZero();
        _matrix._v1y21.toZero(); _matrix._v2y21.toZero();
        _matrix._v1y22.toZero(); _matrix._v2y22.toZero();
        _matrix._v1y0.toZero();  _matrix._v2y0.toZero();
        _matrix._v1zs.toZero();  _matrix._v2zs.toZero();
        _matrix._v1ysh.toZero(); _matrix._v2ysh.toZero();

        refreshBranches();

        if (_onConfigChanged)
            _onConfigChanged();
    }

    void deleteBranch()
    {
        int selRow = _tableBranches.getFirstSelectedRow();
        if (selRow < 0)
        {
            showAlert(td::String("Nema odabira"), td::String("Odaberite vezu koju zelite obrisati."));
            return;
        }

        auto& row = _pDSBranches->getRow(selRow);
        int branchId = row[0].i4Val();

        dp::IStatementPtr pDel(_db->createStatement(
            "DELETE FROM Branches WHERE id = ?"));
        dp::Params par(pDel->allocParams());
        td::Variant idVar(branchId);
        par << idVar;
        pDel->execute();

        refreshBranches();

        if (_onConfigChanged)
            _onConfigChanged();
    }

    bool onChangedSelection(gui::ComboBox* pCmb) override
    {
        if (pCmb == &_conn._cbFrom || pCmb == &_conn._cbTo)
        {
            int fromIdx = _conn._cbFrom.getSelectedIndex();
            int toIdx   = _conn._cbTo.getSelectedIndex();
            if (fromIdx >= 0 && toIdx >= 0)
                updateVisibility(fromIdx == toIdx);
            return true;
        }
        return false;
    }

    bool onClick(gui::Button* pBtn) override
    {
        if (pBtn == &_nodes._btnApplyNodes) { applyNumNodes(); return true; }
        if (pBtn == &_btnAddBranch)         { addBranch();     return true; }
        if (pBtn == &_btnDeleteBranch)      { deleteBranch();  return true; }
        return false;
    }
};

#undef _conn
#undef _matrix
#undef _nodes