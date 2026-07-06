#pragma once
#include <gui/Window.h>
#include <gui/View.h>
#include <gui/VerticalLayout.h>
#include <gui/Button.h>
#include "ViewWithTabs.h"
#include <gui/FileDialog.h>
#include <functional>
// =============================================================================
//  MainWindow.h
// =============================================================================
class MainView : public gui::View
{
public:
    ViewWithTabs     _tabs;
    gui::Button      _btnEstimate;
    gui::VerticalLayout _vl;

    MainView()
    : _btnEstimate("Estimiraj Stanje")
    , _vl(2)
    {
        _vl.append(_tabs);
        _vl.append(_btnEstimate);
        setLayout(&_vl);
    }
};

class MainWindow : public gui::Window
{
protected:
    MainView      _mainView;

public:
    MainWindow()
    : gui::Window(gui::Geometry(50, 50, 900, 900))
    {
        _mainView._btnEstimate.forwardMessagesTo(this);
        setCentralView(&_mainView);
    }

    bool shouldClose() override { return true; }
    void onClose() override { gui::Window::onClose(); }

    bool onClick(gui::Button* pBtn) override
    {
        if (pBtn == &_mainView._btnEstimate)
        {
            _mainView._tabs.estimateState();
            return true;
        }
        return false;
    }

    void saveFile(gui::FileDialog* pDlg)
    {
        if (pDlg->getStatus() == gui::FileDialog::Status::OK)
            mu::dbgLog("Snimanje fajla: %s", pDlg->getFileName().c_str());
        else
            mu::dbgLog("Korisnik odustao od snimanja!");
    }

    void openFile(gui::FileDialog* pDlg)
    {
        if (pDlg->getStatus() == gui::FileDialog::Status::OK)
            mu::dbgLog("Otvaranje fajla: %s", pDlg->getFileName().c_str());
        else
            mu::dbgLog("Korisnik odustao od otvaranja!");
    }

    bool onActionItem(gui::ActionItemDescriptor& aiDesc) override
    {
        auto [menuID, firstSubMenuID, lastSubMenuID, actionID] = aiDesc.getIDs();

        if (menuID == 10 && firstSubMenuID == 0 && lastSubMenuID == 0)
        {
            switch (actionID)
            {
                case 10:
                {
                    td::String dlgTitle("Snimi nesto");
                    auto fnSave = std::bind(&MainWindow::saveFile, this, std::placeholders::_1);
                    gui::SaveFileDialog::show(this, dlgTitle, ".xml", 10, fnSave, "", "ImeFajla");
                    return true;
                }
                case 20:
                {
                    td::String dlgTitle("Otvori nesto");
                    auto fnOpen = std::bind(&MainWindow::openFile, this, std::placeholders::_1);
                    gui::OpenFileDialog::show(this, dlgTitle, {{"XML fajlovi", "*.xml"}, {"7z arhive", "*.7z"}}, 20, fnOpen);
                    return true;
                }
                default:
                    return false;
            }
        }
        return false;
    }
};