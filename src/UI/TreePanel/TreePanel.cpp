// ============================================================================
// TreePanel.cpp
// ============================================================================
#include "TreePanel.h"

// Unicode characters used for UI:
//   ▾ U+25BE  BLACK DOWN-POINTING SMALL TRIANGLE  (expanded)
//   ▸ U+25B8  BLACK RIGHT-POINTING SMALL TRIANGLE  (collapsed)
//   ☑ U+2611  BALLOT BOX WITH CHECK
//   ☐ U+2610  BALLOT BOX

static const wchar_t* const ARROW_OPEN   = L"\u25BE ";
static const wchar_t* const ARROW_CLOSED = L"\u25B8 ";
static const wchar_t* const CHECK_ON     = L"\u2611 ";
static const wchar_t* const CHECK_OFF    = L"\u2610 ";

// ────────────────────────────────────────────────────────────────────────────

int TreePanel::indentSpaces(int level) {
    // level 0 → 0 spaces (section headers at root)
    // level 1 → 2 spaces
    // level 2 → 6 spaces
    // level 3 → 10 spaces
    switch (level) {
        case 1:  return 2;
        case 2:  return 6;
        case 3:  return 10;
        default: return level > 3 ? 10 + (level - 3) * 4 : 0;
    }
}

std::wstring TreePanel::makeCheckText(bool checked, int indent, const wchar_t* name) {
    std::wstring s;
    int sp = indentSpaces(indent);
    if (sp > 0) s.assign((size_t)sp, L' ');
    s += checked ? CHECK_ON : CHECK_OFF;
    s += name;
    return s;
}

std::wstring TreePanel::makeExpandText(bool expanded, int indent, const wchar_t* name) {
    std::wstring s;
    int sp = indentSpaces(indent);
    if (sp > 0) s.assign((size_t)sp, L' ');
    s += expanded ? ARROW_OPEN : ARROW_CLOSED;
    s += name;
    return s;
}

// ────────────────────────────────────────────────────────────────────────────

TreePanel::TreePanel(HWND hListbox)
    : m_hLB(hListbox) {}

void TreePanel::pushItem(const std::wstring& text, Node node) {
    SendMessageW(m_hLB, LB_ADDSTRING, 0, (LPARAM)text.c_str());
    m_nodes.push_back(std::move(node));
}

void TreePanel::clear() {
    SendMessageW(m_hLB, LB_RESETCONTENT, 0, 0);
    m_nodes.clear();
}

void TreePanel::addSection(const wchar_t* name, bool* expanded) {
    std::wstring text = makeExpandText(*expanded, 0, name);
    pushItem(text, { Node::Section, expanded, {} });
}

void TreePanel::addExpandGroup(const wchar_t* name, bool* expanded, int indentLevel) {
    std::wstring text = makeExpandText(*expanded, indentLevel, name);
    pushItem(text, { Node::ExpandGroup, expanded, {} });
}

void TreePanel::addItem(const wchar_t* name, bool* flag, int indentLevel,
                        std::function<void()> onToggle) {
    std::wstring text = makeCheckText(*flag, indentLevel, name);
    Node n;
    n.type = Node::FlagItem;
    n.flag = flag;
    n.callback = std::move(onToggle);
    pushItem(text, std::move(n));
}

void TreePanel::addActionItem(const wchar_t* name, bool active,
                              std::function<void()> onAction, int indentLevel) {
    std::wstring text = makeCheckText(active, indentLevel, name);
    Node n;
    n.type = Node::ActionItem;
    n.flag = nullptr;
    n.callback = std::move(onAction);
    pushItem(text, std::move(n));
}

bool TreePanel::handleClick(int index) {
    // Deselect visual selection — tree panel is click-only, not a selector.
    SendMessageW(m_hLB, LB_SETCURSEL, (WPARAM)-1, 0);

    if (index < 0 || index >= (int)m_nodes.size())
        return false;

    auto& n = m_nodes[(size_t)index];

    switch (n.type) {
        case Node::Section:
        case Node::ExpandGroup:
            if (n.flag) {
                *n.flag = !(*n.flag);
                return true;  // caller should rebuild
            }
            return false;

        case Node::FlagItem:
            if (n.flag) {
                *n.flag = !(*n.flag);
                if (n.callback) n.callback();
                return true;
            }
            return false;

        case Node::ActionItem:
            if (n.callback) n.callback();
            // Action items manage their own state; signal the caller to rebuild.
            return true;
    }
    return false;
}

bool* TreePanel::getClickedFlag(int index) const {
    if (index < 0 || index >= (int)m_nodes.size())
        return nullptr;
    return m_nodes[(size_t)index].flag;
}
