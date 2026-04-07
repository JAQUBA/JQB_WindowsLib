#pragma once

#include <windows.h>
#include <functional>
#include <string>
#include <vector>

// ============================================================================
// TreePanel — LISTBOX-based checkable tree widget with collapsible sections.
//
// Usage:
//   TreePanel* panel = new TreePanel(hListbox);
//
//   // Rebuild is a simple sequence of add* calls between clear() calls:
//   void myRebuild() {
//       panel->clear();
//       panel->addSection(L"Board", &boardExpanded);
//       if (boardExpanded)
//           panel->addItem(L"Outline", &vis.outline);
//       panel->addSection(L"Layers", &layersExpanded);
//       if (layersExpanded) {
//           panel->addItem(L"Copper Top", &vis.copperTop);
//           panel->addExpandGroup(L"Sub-layers", &subExpanded, 1);
//           if (subExpanded) {
//               panel->addItem(L"Traces", &vis.traces, 2);
//               panel->addItem(L"Pads",   &vis.pads,   2,
//                             []{ doRecompute(); });
//           }
//       }
//       panel->addActionItem(L"Drill Only", isDrillOnly,
//                            []{ switchToDrillOnlyMode(); });
//   }
//
//   // In the WM_COMMAND / LBN_SELCHANGE handler:
//   if (panel->handleClick(idx))
//       myRebuild();   // state changed — rebuild the tree
//
// Indent levels:
//   0 = section / top-level item
//   1 = child item       (adds leading spaces)
//   2 = grandchild item  (adds more leading spaces)
//   3 = great-grandchild (maximum)
//
// ============================================================================

class TreePanel {
public:
    explicit TreePanel(HWND hListbox);

    // --- Build API ---

    // Clear all items from the listbox and the internal mapping.
    void clear();

    // Collapsible section header at indent level 0.
    // Clicking toggles *expanded (and the caller should call clear+add* to rebuild).
    void addSection(const wchar_t* name, bool* expanded);

    // Sub-expandable group at indentLevel > 0 (same semantics as addSection but indented).
    void addExpandGroup(const wchar_t* name, bool* expanded, int indentLevel = 1);

    // Checkbox item bound to a bool pointer.
    // onToggle: optional callback invoked AFTER the flag is flipped;
    //           useful for side-effects like cache invalidation.
    void addItem(const wchar_t* name, bool* flag, int indentLevel = 1,
                 std::function<void()> onToggle = nullptr);

    // Item that shows a checkbox based on `active` but fires `onAction` on click.
    // The action is responsible for updating whatever state controls `active`.
    void addActionItem(const wchar_t* name, bool active,
                       std::function<void()> onAction, int indentLevel = 1);

    // --- Event handling ---

    // Call from LBN_SELCHANGE.  Returns true if internal state changed and
    // the caller should rebuild (call clear() + add*() again).
    // Also deselects the item (listbox is used as a click target, not a selector).
    bool handleClick(int listboxIndex);

    // Returns the bool* flag of the clicked item, or nullptr if the item has
    // no flag (section / action).  Useful when the caller needs to identify
    // which flag changed for application-specific side-effects.
    bool* getClickedFlag(int listboxIndex) const;

private:
    HWND m_hLB;

    struct Node {
        enum Type { Section, ExpandGroup, FlagItem, ActionItem } type;
        bool* flag     = nullptr;               // Section / ExpandGroup / FlagItem
        std::function<void()> callback;          // ActionItem action OR FlagItem onToggle
    };

    std::vector<Node> m_nodes;

    void pushItem(const std::wstring& text, Node node);
    static std::wstring makeCheckText(bool checked, int indent, const wchar_t* name);
    static std::wstring makeExpandText(bool expanded, int indent, const wchar_t* name);
    static int indentSpaces(int level);
};
