#pragma once
#include "ViewConfig.h"
#include "ViewMeas.h"
#include "ViewResults.h"

// =============================================================================
//  ViewWithTabs.h
// =============================================================================
 
class ViewWithTabs : public gui::StandardTabView
{
protected:
    ViewConfig    _viewConfig;
    ViewMeas      _viewMeas;
    ViewResults   _viewResults;
 
public:
    ViewWithTabs()
    {
        addView(&_viewConfig, "Konfiguracija", nullptr);
        addView(&_viewMeas,   "Mjerenja",      nullptr);
        addView(&_viewResults,"Rezultati",     nullptr);
 
        _viewMeas.setResultsView(&_viewResults);
 
        _viewConfig.setConfigChangedCallback(
            [this]()
            {
                refreshViewMeas();
            });
 
        refreshViewMeas();
    }
 
    void refreshViewMeas()
    {
        size_t n = _viewConfig.getNumNodes();
        std::vector<std::pair<int, td::String>> nodes;
        for (int i = 1; i <= (int)n; ++i)
            nodes.push_back({i, _viewConfig.getNodeType(i)});
        _viewMeas.setNodeList(nodes);
    }
 
    void estimateState()
    {
        _viewMeas.estimateState();
        setCurrentViewPos(2);
    }
};