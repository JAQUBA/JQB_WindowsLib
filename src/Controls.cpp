#include "Controls.h"
#include <windowsx.h>
#include <commctrl.h>

// Implementacja własnych metod pomocniczych zamiast makr
namespace {
    inline bool CustomSetItemText(HWND hwndLV, int i, int iSubItem, LPWSTR pszText) {
        LVITEM lvi = {0};
        lvi.iSubItem = iSubItem;
        lvi.pszText = pszText;
        return SendMessage(hwndLV, LVM_SETITEMTEXT, (WPARAM)i, (LPARAM)&lvi) != 0;
    }

    inline bool CustomSetItemState(HWND hwndLV, int i, UINT data, UINT mask) {
        LVITEM lvi = {0};
        lvi.state = data;
        lvi.stateMask = mask;
        return SendMessage(hwndLV, LVM_SETITEMSTATE, (WPARAM)i, (LPARAM)&lvi) != 0;
    }
}

// Inicjalizacja Common Controls
namespace {
    // Jednorazowa inicjalizacja kontrolek
    struct CommonControlsInit {
        CommonControlsInit() {
            INITCOMMONCONTROLSEX icc;
            icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
            icc.dwICC = ICC_WIN95_CLASSES | ICC_BAR_CLASSES | ICC_TAB_CLASSES | ICC_LISTVIEW_CLASSES | ICC_TREEVIEW_CLASSES;
            InitCommonControlsEx(&icc);
        }
    };

    // Obiekt statyczny inicjalizujący kontrolki podczas startu programu
    static CommonControlsInit s_commonControlsInit;
}

namespace WinAppLib {

// --- Control Implementation ---

Control::Control(HWND parent, int id) 
    : m_parent(parent)
    , m_id(id)
{
    m_hwnd = GetDlgItem(parent, id);
}

Control::~Control() {
    // Nic specjalnego do zrobienia - nie usuwamy kontrolki, którą nie stworzyliśmy
}

void Control::setText(const std::wstring& text) {
    if (m_hwnd) {
        SetWindowText(m_hwnd, text.c_str());
    }
}

std::wstring Control::getText() const {
    if (!m_hwnd) {
        return L"";
    }
    
    int length = GetWindowTextLength(m_hwnd);
    if (length == 0) {
        return L"";
    }
    
    std::wstring text;
    text.resize(length + 1);
    GetWindowText(m_hwnd, &text[0], length + 1);
    text.resize(length); // Usuń dodatkowy znak NULL
    
    return text;
}

void Control::enable(bool enable) {
    if (m_hwnd) {
        EnableWindow(m_hwnd, enable);
    }
}

bool Control::isEnabled() const {
    if (!m_hwnd) {
        return false;
    }
    
    return IsWindowEnabled(m_hwnd) != FALSE;
}

void Control::show(bool show) {
    if (m_hwnd) {
        ShowWindow(m_hwnd, show ? SW_SHOW : SW_HIDE);
    }
}

bool Control::isVisible() const {
    if (!m_hwnd) {
        return false;
    }
    
    return IsWindowVisible(m_hwnd) != FALSE;
}

// --- Button Implementation ---

Button::Button(HWND parent, int id) 
    : Control(parent, id)
{
    // Korzystamy z konstruktora bazowego
}

Button::Button(HWND parent, int id, const std::wstring& text, int x, int y, int width, int height, DWORD style)
    : Control(parent, id)
{
    // Tworzymy nową kontrolkę
    m_hwnd = CreateWindowEx(
        0,                      // Rozszerzony styl
        L"BUTTON",              // Klasa kontrolki
        text.c_str(),           // Tekst przycisku
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | style, // Style
        x, y, width, height,    // Rozmiar i pozycja
        parent,                 // Rodzic
        (HMENU)(INT_PTR)id,     // ID kontrolki
        (HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE), // Instancja
        NULL                    // Parametry dodatkowe
    );
}

Button::~Button() {
    // Nic specjalnego do zrobienia
}

void Button::setClickHandler(std::function<void()> callback) {
    m_clickCallback = callback;
}

bool Button::handleCommand(WPARAM wParam, LPARAM lParam) {
    // Sprawdź, czy komunikat dotyczy tej kontrolki
    if (LOWORD(wParam) == m_id && HIWORD(wParam) == BN_CLICKED) {
        if (m_clickCallback) {
            m_clickCallback();
            return true;
        }
    }
    
    return false;
}

// --- TextBox Implementation ---

TextBox::TextBox(HWND parent, int id)
    : Control(parent, id)
{
    // Korzystamy z konstruktora bazowego
}

TextBox::TextBox(HWND parent, int id, const std::wstring& text, int x, int y, int width, int height, DWORD style)
    : Control(parent, id)
{
    // Tworzymy nową kontrolkę
    m_hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,       // Rozszerzony styl dla obramowania
        L"EDIT",                // Klasa kontrolki
        text.c_str(),           // Tekst początkowy
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | style, // Style
        x, y, width, height,    // Rozmiar i pozycja
        parent,                 // Rodzic
        (HMENU)(INT_PTR)id,     // ID kontrolki
        (HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE), // Instancja
        NULL                    // Parametry dodatkowe
    );
}

TextBox::~TextBox() {
    // Nic specjalnego do zrobienia
}

void TextBox::setText(const std::wstring& text) {
    if (m_hwnd) {
        SetWindowText(m_hwnd, text.c_str());
    }
}

void TextBox::appendText(const std::wstring& text) {
    if (!m_hwnd) {
        return;
    }
    
    // Pobierz długość aktualnego tekstu
    int length = GetWindowTextLength(m_hwnd);
    
    // Wybierz pozycję na końcu tekstu
    SendMessage(m_hwnd, EM_SETSEL, (WPARAM)length, (LPARAM)length);
    
    // Wstaw nowy tekst
    SendMessage(m_hwnd, EM_REPLACESEL, FALSE, (LPARAM)text.c_str());
}

void TextBox::clear() {
    if (m_hwnd) {
        SetWindowText(m_hwnd, L"");
    }
}

void TextBox::selectAll() {
    if (m_hwnd) {
        SendMessage(m_hwnd, EM_SETSEL, 0, -1);
    }
}

void TextBox::scrollToEnd() {
    if (m_hwnd) {
        SendMessage(m_hwnd, WM_VSCROLL, SB_BOTTOM, 0);
    }
}

void TextBox::setReadOnly(bool readOnly) {
    if (m_hwnd) {
        SendMessage(m_hwnd, EM_SETREADONLY, readOnly, 0);
    }
}

void TextBox::setChangeHandler(std::function<void(const std::wstring&)> callback) {
    m_changeCallback = callback;
}

bool TextBox::handleCommand(WPARAM wParam, LPARAM lParam) {
    // Sprawdź, czy komunikat dotyczy tej kontrolki
    if (LOWORD(wParam) == m_id && HIWORD(wParam) == EN_CHANGE) {
        if (m_changeCallback) {
            m_changeCallback(getText());
            return true;
        }
    }
    
    return false;
}

// --- ComboBox Implementation ---

ComboBox::ComboBox(HWND parent, int id)
    : Control(parent, id)
{
    // Korzystamy z konstruktora bazowego
}

ComboBox::ComboBox(HWND parent, int id, int x, int y, int width, int height, DWORD style)
    : Control(parent, id)
{
    // Tworzymy nową kontrolkę
    m_hwnd = CreateWindowEx(
        0,                      // Rozszerzony styl
        L"COMBOBOX",            // Klasa kontrolki
        L"",                    // Brak początkowego tekstu
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL | style, // Style
        x, y, width, height,    // Rozmiar i pozycja
        parent,                 // Rodzic
        (HMENU)(INT_PTR)id,     // ID kontrolki
        (HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE), // Instancja
        NULL                    // Parametry dodatkowe
    );
}

ComboBox::~ComboBox() {
    // Nic specjalnego do zrobienia
}

int ComboBox::addItem(const std::wstring& text, LPARAM data) {
    if (!m_hwnd) {
        return CB_ERR;
    }
    
    int index = (int)SendMessage(m_hwnd, CB_ADDSTRING, 0, (LPARAM)text.c_str());
    if (index != CB_ERR && data != 0) {
        SendMessage(m_hwnd, CB_SETITEMDATA, index, data);
    }
    
    return index;
}

int ComboBox::insertItem(int index, const std::wstring& text, LPARAM data) {
    if (!m_hwnd) {
        return CB_ERR;
    }
    
    int result = (int)SendMessage(m_hwnd, CB_INSERTSTRING, index, (LPARAM)text.c_str());
    if (result != CB_ERR && data != 0) {
        SendMessage(m_hwnd, CB_SETITEMDATA, result, data);
    }
    
    return result;
}

bool ComboBox::removeItem(int index) {
    if (!m_hwnd) {
        return false;
    }
    
    return SendMessage(m_hwnd, CB_DELETESTRING, index, 0) != CB_ERR;
}

void ComboBox::clear() {
    if (m_hwnd) {
        SendMessage(m_hwnd, CB_RESETCONTENT, 0, 0);
    }
}

bool ComboBox::selectItem(int index) {
    if (!m_hwnd) {
        return false;
    }
    
    return SendMessage(m_hwnd, CB_SETCURSEL, index, 0) != CB_ERR;
}

int ComboBox::getSelectedIndex() const {
    if (!m_hwnd) {
        return CB_ERR;
    }
    
    return (int)SendMessage(m_hwnd, CB_GETCURSEL, 0, 0);
}

std::wstring ComboBox::getSelectedText() const {
    int index = getSelectedIndex();
    if (index == CB_ERR) {
        return L"";
    }
    
    return getItemText(index);
}

LPARAM ComboBox::getSelectedData() const {
    int index = getSelectedIndex();
    if (index == CB_ERR) {
        return 0;
    }
    
    return getItemData(index);
}

int ComboBox::getCount() const {
    if (!m_hwnd) {
        return 0;
    }
    
    return (int)SendMessage(m_hwnd, CB_GETCOUNT, 0, 0);
}

std::wstring ComboBox::getItemText(int index) const {
    if (!m_hwnd || index < 0) {
        return L"";
    }
    
    int length = (int)SendMessage(m_hwnd, CB_GETLBTEXTLEN, index, 0);
    if (length == CB_ERR) {
        return L"";
    }
    
    std::wstring text;
    text.resize(length + 1);
    SendMessage(m_hwnd, CB_GETLBTEXT, index, (LPARAM)&text[0]);
    text.resize(length);  // Usuń dodatkowy znak NULL
    
    return text;
}

LPARAM ComboBox::getItemData(int index) const {
    if (!m_hwnd || index < 0) {
        return 0;
    }
    
    return SendMessage(m_hwnd, CB_GETITEMDATA, index, 0);
}

void ComboBox::setSelectionChangeHandler(std::function<void(int)> callback) {
    m_selectionChangeCallback = callback;
}

bool ComboBox::handleCommand(WPARAM wParam, LPARAM lParam) {
    // Sprawdź, czy komunikat dotyczy tej kontrolki
    if (LOWORD(wParam) == m_id && HIWORD(wParam) == CBN_SELCHANGE) {
        if (m_selectionChangeCallback) {
            m_selectionChangeCallback(getSelectedIndex());
            return true;
        }
    }
    
    return false;
}

// --- Label Implementation ---

Label::Label(HWND parent, int id)
    : Control(parent, id)
{
    // Korzystamy z konstruktora bazowego
}

Label::Label(HWND parent, int id, const std::wstring& text, int x, int y, int width, int height, DWORD style)
    : Control(parent, id)
{
    // Tworzymy nową kontrolkę
    m_hwnd = CreateWindowEx(
        0,                      // Rozszerzony styl
        L"STATIC",              // Klasa kontrolki
        text.c_str(),           // Tekst początkowy
        WS_CHILD | WS_VISIBLE | SS_LEFT | style, // Style
        x, y, width, height,    // Rozmiar i pozycja
        parent,                 // Rodzic
        (HMENU)(INT_PTR)id,     // ID kontrolki
        (HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE), // Instancja
        NULL                    // Parametry dodatkowe
    );
}

Label::~Label() {
    // Nic specjalnego do zrobienia
}

void Label::setTextAlign(DWORD align) {
    if (m_hwnd) {
        LONG_PTR style = GetWindowLongPtr(m_hwnd, GWL_STYLE);
        style &= ~(SS_LEFT | SS_CENTER | SS_RIGHT); // Usuń aktualne wyrównanie
        style |= align;  // Dodaj nowe wyrównanie
        SetWindowLongPtr(m_hwnd, GWL_STYLE, style);
        InvalidateRect(m_hwnd, NULL, TRUE); // Odśwież kontrolkę
    }
}

// --- CheckBox Implementation ---

CheckBox::CheckBox(HWND parent, int id)
    : Control(parent, id)
{
    // Korzystamy z konstruktora bazowego
}

CheckBox::CheckBox(HWND parent, int id, const std::wstring& text, int x, int y, int width, int height, DWORD style)
    : Control(parent, id)
{
    // Tworzymy nową kontrolkę
    m_hwnd = CreateWindowEx(
        0,                      // Rozszerzony styl
        L"BUTTON",              // Klasa kontrolki
        text.c_str(),           // Tekst początkowy
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | style, // Style
        x, y, width, height,    // Rozmiar i pozycja
        parent,                 // Rodzic
        (HMENU)(INT_PTR)id,     // ID kontrolki
        (HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE), // Instancja
        NULL                    // Parametry dodatkowe
    );
}

CheckBox::~CheckBox() {
    // Nic specjalnego do zrobienia
}

void CheckBox::setChecked(bool checked) {
    if (m_hwnd) {
        SendMessage(m_hwnd, BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED, 0);
    }
}

bool CheckBox::isChecked() const {
    if (!m_hwnd) {
        return false;
    }
    
    return SendMessage(m_hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED;
}

void CheckBox::setStateChangeHandler(std::function<void(bool)> callback) {
    m_stateChangeCallback = callback;
}

bool CheckBox::handleCommand(WPARAM wParam, LPARAM lParam) {
    // Sprawdź, czy komunikat dotyczy tej kontrolki
    if (LOWORD(wParam) == m_id && HIWORD(wParam) == BN_CLICKED) {
        if (m_stateChangeCallback) {
            m_stateChangeCallback(isChecked());
            return true;
        }
    }
    
    return false;
}

// === RadioButton ===

RadioButton::RadioButton(HWND parent, int id)
    : Control(parent, id)
{
    // Korzystamy z konstruktora bazowego
}

RadioButton::RadioButton(HWND parent, int id, const std::wstring& text, int x, int y, int width, int height, DWORD style)
    : Control(parent, id)
{
    // Tworzymy nową kontrolkę
    m_hwnd = CreateWindowEx(
        0,                      // Rozszerzony styl
        L"BUTTON",              // Klasa kontrolki
        text.c_str(),           // Tekst przycisku
        WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | style, // Style
        x, y, width, height,    // Rozmiar i pozycja
        parent,                 // Rodzic
        (HMENU)(INT_PTR)id,     // ID kontrolki
        (HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE), // Instancja
        NULL                    // Parametry dodatkowe
    );
}

RadioButton::~RadioButton() {
    // Nic specjalnego do zrobienia
}

void RadioButton::setChecked(bool checked) {
    if (m_hwnd) {
        SendMessage(m_hwnd, BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED, 0);
    }
}

bool RadioButton::isChecked() const {
    if (!m_hwnd) {
        return false;
    }
    
    return SendMessage(m_hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED;
}

void RadioButton::setStateChangeHandler(std::function<void()> callback) {
    m_stateChangeCallback = callback;
}

bool RadioButton::handleCommand(WPARAM wParam, LPARAM lParam) {
    // Sprawdź, czy komunikat dotyczy tej kontrolki
    if (LOWORD(wParam) == m_id && HIWORD(wParam) == BN_CLICKED) {
        if (m_stateChangeCallback && isChecked()) {
            m_stateChangeCallback();
            return true;
        }
    }
    
    return false;
}

// === GroupBox ===

GroupBox::GroupBox(HWND parent, int id)
    : Control(parent, id)
{
    // Korzystamy z konstruktora bazowego
}

GroupBox::GroupBox(HWND parent, int id, const std::wstring& text, int x, int y, int width, int height, DWORD style)
    : Control(parent, id)
{
    // Tworzymy nową kontrolkę
    m_hwnd = CreateWindowEx(
        0,                      // Rozszerzony styl
        L"BUTTON",              // Klasa kontrolki
        text.c_str(),           // Tekst etykiety
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX | style, // Style
        x, y, width, height,    // Rozmiar i pozycja
        parent,                 // Rodzic
        (HMENU)(INT_PTR)id,     // ID kontrolki
        (HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE), // Instancja
        NULL                    // Parametry dodatkowe
    );
}

GroupBox::~GroupBox() {
    // Nic specjalnego do zrobienia
}

// === ProgressBar ===

ProgressBar::ProgressBar(HWND parent, int id)
    : Control(parent, id)
{
    // Korzystamy z konstruktora bazowego
}

ProgressBar::ProgressBar(HWND parent, int id, int x, int y, int width, int height, DWORD style)
    : Control(parent, id)
{
    // Tworzymy nową kontrolkę
    m_hwnd = CreateWindowEx(
        0,                      // Rozszerzony styl
        PROGRESS_CLASS,         // Klasa kontrolki
        L"",                    // Brak tekstu
        WS_CHILD | WS_VISIBLE | style, // Style
        x, y, width, height,    // Rozmiar i pozycja
        parent,                 // Rodzic
        (HMENU)(INT_PTR)id,     // ID kontrolki
        (HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE), // Instancja
        NULL                    // Parametry dodatkowe
    );
}

ProgressBar::~ProgressBar() {
    // Nic specjalnego do zrobienia
}

void ProgressBar::setRange(int minValue, int maxValue) {
    if (m_hwnd) {
        SendMessage(m_hwnd, PBM_SETRANGE32, minValue, maxValue);
    }
}

int ProgressBar::getMinRange() const {
    if (!m_hwnd) {
        return 0;
    }
    
    PBRANGE range = {0};
    SendMessage(m_hwnd, PBM_GETRANGE, TRUE, (LPARAM)&range);
    return range.iLow;
}

int ProgressBar::getMaxRange() const {
    if (!m_hwnd) {
        return 0;
    }
    
    PBRANGE range = {0};
    SendMessage(m_hwnd, PBM_GETRANGE, FALSE, (LPARAM)&range);
    return range.iHigh;
}

int ProgressBar::setPosition(int pos) {
    if (!m_hwnd) {
        return 0;
    }
    
    return (int)SendMessage(m_hwnd, PBM_SETPOS, pos, 0);
}

int ProgressBar::getPosition() const {
    if (!m_hwnd) {
        return 0;
    }
    
    return (int)SendMessage(m_hwnd, PBM_GETPOS, 0, 0);
}

int ProgressBar::deltaPosition(int increment) {
    if (!m_hwnd) {
        return 0;
    }
    
    return (int)SendMessage(m_hwnd, PBM_DELTAPOS, increment, 0);
}

void ProgressBar::setState(int state) {
    if (m_hwnd) {
        SendMessage(m_hwnd, PBM_SETSTATE, state, 0);
    }
}

void ProgressBar::setBarColor(COLORREF color) {
    if (m_hwnd) {
        SendMessage(m_hwnd, PBM_SETBARCOLOR, 0, color);
    }
}

void ProgressBar::setBkColor(COLORREF color) {
    if (m_hwnd) {
        SendMessage(m_hwnd, PBM_SETBKCOLOR, 0, color);
    }
}

// === TrackBar (Slider) ===

TrackBar::TrackBar(HWND parent, int id)
    : Control(parent, id)
{
    // Korzystamy z konstruktora bazowego
}

TrackBar::TrackBar(HWND parent, int id, int x, int y, int width, int height, DWORD style)
    : Control(parent, id)
{
    // Tworzymy nową kontrolkę
    m_hwnd = CreateWindowEx(
        0,                      // Rozszerzony styl
        TRACKBAR_CLASS,         // Klasa kontrolki
        L"",                    // Brak tekstu
        WS_CHILD | WS_VISIBLE | TBS_HORZ | style, // Style
        x, y, width, height,    // Rozmiar i pozycja
        parent,                 // Rodzic
        (HMENU)(INT_PTR)id,     // ID kontrolki
        (HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE), // Instancja
        NULL                    // Parametry dodatkowe
    );
}

TrackBar::~TrackBar() {
    // Nic specjalnego do zrobienia
}

void TrackBar::setRange(int minValue, int maxValue) {
    if (m_hwnd) {
        SendMessage(m_hwnd, TBM_SETRANGE, TRUE, MAKELPARAM(minValue, maxValue));
    }
}

int TrackBar::getMinRange() const {
    if (!m_hwnd) {
        return 0;
    }
    
    return (int)SendMessage(m_hwnd, TBM_GETRANGEMIN, 0, 0);
}

int TrackBar::getMaxRange() const {
    if (!m_hwnd) {
        return 0;
    }
    
    return (int)SendMessage(m_hwnd, TBM_GETRANGEMAX, 0, 0);
}

int TrackBar::setPosition(int pos) {
    if (!m_hwnd) {
        return 0;
    }
    
    return (int)SendMessage(m_hwnd, TBM_SETPOS, TRUE, pos);
}

int TrackBar::getPosition() const {
    if (!m_hwnd) {
        return 0;
    }
    
    return (int)SendMessage(m_hwnd, TBM_GETPOS, 0, 0);
}

void TrackBar::setTickFrequency(int frequency) {
    if (m_hwnd) {
        SendMessage(m_hwnd, TBM_SETTICFREQ, frequency, 0);
    }
}

void TrackBar::showTicks(bool show) {
    if (!m_hwnd) {
        return;
    }
    
    LONG_PTR style = GetWindowLongPtr(m_hwnd, GWL_STYLE);
    if (show) {
        style |= TBS_AUTOTICKS;
    } else {
        style &= ~TBS_AUTOTICKS;
    }
    SetWindowLongPtr(m_hwnd, GWL_STYLE, style);
    InvalidateRect(m_hwnd, NULL, TRUE);
}

void TrackBar::setPositionChangeHandler(std::function<void(int)> callback) {
    m_positionChangeCallback = callback;
}

bool TrackBar::handleNotify(WPARAM wParam, LPARAM lParam) {
    NMHDR* pNMHDR = (NMHDR*)lParam;
    if (pNMHDR->hwndFrom == m_hwnd && pNMHDR->code == NM_RELEASEDCAPTURE) {
        if (m_positionChangeCallback) {
            m_positionChangeCallback(getPosition());
            return true;
        }
    }
    
    return false;
}

// === StatusBar ===

StatusBar::StatusBar(HWND parent, int id)
    : Control(parent, id)
{
    // Korzystamy z konstruktora bazowego
}

StatusBar::StatusBar(HWND parent, int id, DWORD style)
    : Control(parent, id)
{
    // Tworzymy nową kontrolkę
    m_hwnd = CreateWindowEx(
        0,                      // Rozszerzony styl
        STATUSCLASSNAME,        // Klasa kontrolki
        L"",                    // Brak tekstu
        WS_CHILD | WS_VISIBLE | style, // Style
        0, 0, 0, 0,             // Rozmiar i pozycja - automatycznie dostosowywane
        parent,                 // Rodzic
        (HMENU)(INT_PTR)id,     // ID kontrolki
        (HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE), // Instancja
        NULL                    // Parametry dodatkowe
    );

    if (m_hwnd) {
        // Domyślnie jedna część na całą szerokość
        int widths[1] = {-1};
        SendMessage(m_hwnd, SB_SETPARTS, 1, (LPARAM)widths);
    }
}

StatusBar::~StatusBar() {
    // Nic specjalnego do zrobienia
}

void StatusBar::setText(int partIndex, const std::wstring& text) {
    if (m_hwnd) {
        SendMessage(m_hwnd, SB_SETTEXT, partIndex, (LPARAM)text.c_str());
    }
}

void StatusBar::setText(const std::wstring& text) {
    setText(0, text);
}

std::wstring StatusBar::getText(int partIndex) const {
    if (!m_hwnd) {
        return L"";
    }
    
    int textLength = (int)SendMessage(m_hwnd, SB_GETTEXTLENGTH, partIndex, 0);
    if (textLength <= 0) {
        return L"";
    }
    
    std::wstring text;
    text.resize(textLength + 1);
    SendMessage(m_hwnd, SB_GETTEXT, partIndex, (LPARAM)&text[0]);
    text.resize(textLength); // Usuń dodatkowy znak NULL
    
    return text;
}

void StatusBar::setIcons(HICON* icons, int count) {
    if (!m_hwnd || !icons || count <= 0) {
        return;
    }
    
    for (int i = 0; i < count; i++) {
        SendMessage(m_hwnd, SB_SETICON, i, (LPARAM)icons[i]);
    }
}

void StatusBar::setParts(int count, const int* widths) {
    if (!m_hwnd || count <= 0 || !widths) {
        return;
    }
    
    SendMessage(m_hwnd, SB_SETPARTS, count, (LPARAM)widths);
}

int StatusBar::getPartsCount() const {
    if (!m_hwnd) {
        return 0;
    }
    
    return (int)SendMessage(m_hwnd, SB_GETPARTS, 0, 0);
}

void StatusBar::autoSize() {
    if (m_hwnd) {
        SendMessage(m_hwnd, WM_SIZE, 0, 0);
    }
}

// === TabControl ===

TabControl::TabControl(HWND parent, int id)
    : Control(parent, id)
{
    // Korzystamy z konstruktora bazowego
}

TabControl::TabControl(HWND parent, int id, int x, int y, int width, int height, DWORD style)
    : Control(parent, id)
{
    // Tworzymy nową kontrolkę
    m_hwnd = CreateWindowEx(
        0,                      // Rozszerzony styl
        WC_TABCONTROL,          // Klasa kontrolki
        L"",                    // Brak tekstu
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | style, // Style
        x, y, width, height,    // Rozmiar i pozycja
        parent,                 // Rodzic
        (HMENU)(INT_PTR)id,     // ID kontrolki
        (HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE), // Instancja
        NULL                    // Parametry dodatkowe
    );
}

TabControl::~TabControl() {
    // Nic specjalnego do zrobienia
}

int TabControl::addTab(const std::wstring& text, LPARAM param, int imageIndex) {
    if (!m_hwnd) {
        return -1;
    }
    
    TCITEM tie = {0};
    tie.mask = TCIF_TEXT | TCIF_PARAM;
    if (imageIndex >= 0) {
        tie.mask |= TCIF_IMAGE;
        tie.iImage = imageIndex;
    }
    tie.pszText = (LPWSTR)text.c_str();
    tie.lParam = param;
    
    return (int)SendMessage(m_hwnd, TCM_INSERTITEM, 0x7FFFFFFF, (LPARAM)&tie);
}

bool TabControl::deleteTab(int index) {
    if (!m_hwnd || index < 0) {
        return false;
    }
    
    return SendMessage(m_hwnd, TCM_DELETEITEM, index, 0) != 0;
}

bool TabControl::selectTab(int index) {
    if (!m_hwnd || index < 0) {
        return false;
    }
    
    return SendMessage(m_hwnd, TCM_SETCURSEL, index, 0) != -1;
}

int TabControl::getSelectedTabIndex() const {
    if (!m_hwnd) {
        return -1;
    }
    
    return (int)SendMessage(m_hwnd, TCM_GETCURSEL, 0, 0);
}

LPARAM TabControl::getTabParam(int index) const {
    if (!m_hwnd || index < 0) {
        return 0;
    }
    
    TCITEM tie = {0};
    tie.mask = TCIF_PARAM;
    SendMessage(m_hwnd, TCM_GETITEM, index, (LPARAM)&tie);
    return tie.lParam;
}

void TabControl::setTabParam(int index, LPARAM param) {
    if (!m_hwnd || index < 0) {
        return;
    }
    
    TCITEM tie = {0};
    tie.mask = TCIF_PARAM;
    tie.lParam = param;
    SendMessage(m_hwnd, TCM_SETITEM, index, (LPARAM)&tie);
}

std::wstring TabControl::getTabText(int index) const {
    if (!m_hwnd || index < 0) {
        return L"";
    }
    
    // Najpierw sprawdzamy długość tekstu
    TCITEM tie = {0};
    tie.mask = TCIF_TEXT;
    tie.pszText = NULL;
    tie.cchTextMax = 0;
    SendMessage(m_hwnd, TCM_GETITEM, index, (LPARAM)&tie);
    
    // Teraz pobieramy sam tekst
    std::wstring text;
    text.resize(256); // Zakładamy, że tekst nie jest dłuższy niż 256 znaków
    tie.pszText = &text[0];
    tie.cchTextMax = 256;
    SendMessage(m_hwnd, TCM_GETITEM, index, (LPARAM)&tie);
    
    // Usuwamy nadmiarowe znaki zerowe
    size_t length = wcslen(tie.pszText);
    text.resize(length);
    
    return text;
}

void TabControl::setTabText(int index, const std::wstring& text) {
    if (!m_hwnd || index < 0) {
        return;
    }
    
    TCITEM tie = {0};
    tie.mask = TCIF_TEXT;
    tie.pszText = (LPWSTR)text.c_str();
    SendMessage(m_hwnd, TCM_SETITEM, index, (LPARAM)&tie);
}

int TabControl::getTabCount() const {
    if (!m_hwnd) {
        return 0;
    }
    
    return (int)SendMessage(m_hwnd, TCM_GETITEMCOUNT, 0, 0);
}

void TabControl::setImageList(HIMAGELIST imageList) {
    if (m_hwnd) {
        SendMessage(m_hwnd, TCM_SETIMAGELIST, 0, (LPARAM)imageList);
    }
}

void TabControl::setTabChangeHandler(std::function<void(int)> callback) {
    m_tabChangeCallback = callback;
}

bool TabControl::handleNotify(WPARAM wParam, LPARAM lParam) {
    NMHDR* pNMHDR = (NMHDR*)lParam;
    if (pNMHDR->hwndFrom == m_hwnd) {
        switch (pNMHDR->code) {
            case TCN_SELCHANGE:
                if (m_tabChangeCallback) {
                    m_tabChangeCallback(getSelectedTabIndex());
                    return true;
                }
                break;
        }
    }
    
    return false;
}

void TabControl::getClientRect(RECT& rect) const {
    if (!m_hwnd) {
        SetRectEmpty(&rect);
        return;
    }
    
    // Pobierz rozmiar kontrolki
    GetClientRect(m_hwnd, &rect);
    
    // Dostosuj prostokąt, usuwając obszar zakładek
    SendMessage(m_hwnd, TCM_ADJUSTRECT, FALSE, (LPARAM)&rect);
}

// === ListView ===

ListView::ListView(HWND parent, int id)
    : Control(parent, id)
{
    // Korzystamy z konstruktora bazowego
}

ListView::ListView(HWND parent, int id, int x, int y, int width, int height, DWORD style)
    : Control(parent, id)
{
    // Tworzymy nową kontrolkę
    m_hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,       // Rozszerzony styl
        WC_LISTVIEW,            // Klasa kontrolki
        L"",                    // Brak tekstu
        WS_CHILD | WS_VISIBLE | WS_BORDER | style, // Style
        x, y, width, height,    // Rozmiar i pozycja
        parent,                 // Rodzic
        (HMENU)(INT_PTR)id,     // ID kontrolki
        (HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE), // Instancja
        NULL                    // Parametry dodatkowe
    );
    
    // Włączamy domyślnie styl extended dla pełnego wyboru wiersza i linii siatki
    if (m_hwnd) {
        ListView_SetExtendedListViewStyle(m_hwnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    }
}

ListView::~ListView() {
    // Nic specjalnego do zrobienia
}

int ListView::addColumn(int index, const std::wstring& text, int width, int format) {
    if (!m_hwnd) {
        return -1;
    }
    
    LVCOLUMN lvc = {0};
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvc.fmt = format;
    lvc.cx = width;
    lvc.pszText = (LPWSTR)text.c_str();
    lvc.iSubItem = index;
    
    return ListView_InsertColumn(m_hwnd, index, &lvc);
}

bool ListView::deleteColumn(int index) {
    if (!m_hwnd || index < 0) {
        return false;
    }
    
    return ListView_DeleteColumn(m_hwnd, index) != 0;
}

int ListView::addItem(const std::wstring& text, int imageIndex) {
    if (!m_hwnd) {
        return -1;
    }
    
    LVITEM lvi = {0};
    lvi.mask = LVIF_TEXT;
    if (imageIndex >= 0) {
        lvi.mask |= LVIF_IMAGE;
        lvi.iImage = imageIndex;
    }
    lvi.iItem = getItemCount();
    lvi.pszText = (LPWSTR)text.c_str();
    
    return ListView_InsertItem(m_hwnd, &lvi);
}

bool ListView::setItemText(int itemIndex, int subItemIndex, const std::wstring& text) {
    if (!m_hwnd || itemIndex < 0 || subItemIndex < 0) {
        return false;
    }
    
    return CustomSetItemText(m_hwnd, itemIndex, subItemIndex, (LPWSTR)text.c_str());
}

std::wstring ListView::getItemText(int itemIndex, int subItemIndex) const {
    if (!m_hwnd || itemIndex < 0 || subItemIndex < 0) {
        return L"";
    }
    
    std::wstring text;
    text.resize(256); // Zakładamy maksymalną długość tekstu
    ListView_GetItemText(m_hwnd, itemIndex, subItemIndex, &text[0], 256);
    
    // Usuwamy nadmiarowe znaki zerowe
    size_t length = wcslen(text.c_str());
    text.resize(length);
    
    return text;
}

bool ListView::setItemData(int index, LPARAM data) {
    if (!m_hwnd || index < 0) {
        return false;
    }
    
    LVITEM lvi = {0};
    lvi.mask = LVIF_PARAM;
    lvi.iItem = index;
    lvi.lParam = data;
    
    return ListView_SetItem(m_hwnd, &lvi) != 0;
}

LPARAM ListView::getItemData(int index) const {
    if (!m_hwnd || index < 0) {
        return 0;
    }
    
    LVITEM lvi = {0};
    lvi.mask = LVIF_PARAM;
    lvi.iItem = index;
    ListView_GetItem(m_hwnd, &lvi);
    
    return lvi.lParam;
}

bool ListView::deleteItem(int index) {
    if (!m_hwnd || index < 0) {
        return false;
    }
    
    return ListView_DeleteItem(m_hwnd, index) != 0;
}

void ListView::clear() {
    if (m_hwnd) {
        ListView_DeleteAllItems(m_hwnd);
    }
}

bool ListView::selectItem(int index, bool deselectOthers) {
    if (!m_hwnd || index < 0) {
        return false;
    }
    
    if (deselectOthers) {
        CustomSetItemState(m_hwnd, -1, 0, LVIS_SELECTED);
    }
    
    return CustomSetItemState(m_hwnd, index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
}

bool ListView::isItemSelected(int index) const {
    if (!m_hwnd || index < 0) {
        return false;
    }
    
    return (ListView_GetItemState(m_hwnd, index, LVIS_SELECTED) & LVIS_SELECTED) != 0;
}

int ListView::getSelectedIndex() const {
    if (!m_hwnd) {
        return -1;
    }
    
    return ListView_GetNextItem(m_hwnd, -1, LVNI_SELECTED);
}

std::vector<int> ListView::getSelectedIndices() const {
    std::vector<int> indices;
    if (!m_hwnd) {
        return indices;
    }
    
    // Przeszukujemy wszystkie zaznaczone elementy
    int index = -1;
    while ((index = ListView_GetNextItem(m_hwnd, index, LVNI_SELECTED)) != -1) {
        indices.push_back(index);
    }
    
    return indices;
}

int ListView::getItemCount() const {
    if (!m_hwnd) {
        return 0;
    }
    
    return ListView_GetItemCount(m_hwnd);
}

void ListView::setImageList(HIMAGELIST imageList, int type) {
    if (m_hwnd) {
        ListView_SetImageList(m_hwnd, imageList, type);
    }
}

void ListView::enableFullRowSelect(bool enable) {
    if (!m_hwnd) {
        return;
    }
    
    DWORD exStyle = ListView_GetExtendedListViewStyle(m_hwnd);
    if (enable) {
        exStyle |= LVS_EX_FULLROWSELECT;
    } else {
        exStyle &= ~LVS_EX_FULLROWSELECT;
    }
    ListView_SetExtendedListViewStyle(m_hwnd, exStyle);
}

void ListView::enableGridLines(bool enable) {
    if (!m_hwnd) {
        return;
    }
    
    DWORD exStyle = ListView_GetExtendedListViewStyle(m_hwnd);
    if (enable) {
        exStyle |= LVS_EX_GRIDLINES;
    } else {
        exStyle &= ~LVS_EX_GRIDLINES;
    }
    ListView_SetExtendedListViewStyle(m_hwnd, exStyle);
}

void ListView::setItemClickHandler(std::function<void(int)> callback) {
    m_itemClickCallback = callback;
}

void ListView::setItemDoubleClickHandler(std::function<void(int)> callback) {
    m_itemDoubleClickCallback = callback;
}

void ListView::setSelectionChangeHandler(std::function<void()> callback) {
    m_selectionChangeCallback = callback;
}

bool ListView::handleNotify(WPARAM wParam, LPARAM lParam) {
    NMHDR* pNMHDR = (NMHDR*)lParam;
    if (pNMHDR->hwndFrom == m_hwnd) {
        switch (pNMHDR->code) {
            case NM_CLICK: 
                if (m_itemClickCallback) {
                    NMITEMACTIVATE* pNMItem = (NMITEMACTIVATE*)lParam;
                    m_itemClickCallback(pNMItem->iItem);
                    return true;
                }
                break;
                
            case NM_DBLCLK:
                if (m_itemDoubleClickCallback) {
                    NMITEMACTIVATE* pNMItem = (NMITEMACTIVATE*)lParam;
                    m_itemDoubleClickCallback(pNMItem->iItem);
                    return true;
                }
                break;
                
            case LVN_ITEMCHANGED: {
                NMLISTVIEW* pNMLV = (NMLISTVIEW*)lParam;
                if ((pNMLV->uNewState ^ pNMLV->uOldState) & LVIS_SELECTED) {
                    // Zmienił się stan zaznaczenia
                    if (m_selectionChangeCallback) {
                        m_selectionChangeCallback();
                        return true;
                    }
                }
                break;
            }
        }
    }
    
    return false;
}

// === TreeView ===

TreeView::TreeView(HWND parent, int id)
    : Control(parent, id)
{
    // Korzystamy z konstruktora bazowego
}

TreeView::TreeView(HWND parent, int id, int x, int y, int width, int height, DWORD style)
    : Control(parent, id)
{
    // Tworzymy nową kontrolkę
    m_hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,       // Rozszerzony styl
        WC_TREEVIEW,            // Klasa kontrolki
        L"",                    // Brak tekstu
        WS_CHILD | WS_VISIBLE | WS_BORDER | style, // Style
        x, y, width, height,    // Rozmiar i pozycja
        parent,                 // Rodzic
        (HMENU)(INT_PTR)id,     // ID kontrolki
        (HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE), // Instancja
        NULL                    // Parametry dodatkowe
    );
}

TreeView::~TreeView() {
    // Nic specjalnego do zrobienia
}

TreeView::TreeItem TreeView::addItem(const std::wstring& text, TreeItem parent, TreeItem insertAfter, 
                                 int imageIndex, int selectedImageIndex, LPARAM param) {
    if (!m_hwnd) {
        return NULL;
    }
    
    TV_INSERTSTRUCT tvis = {0};
    tvis.hParent = parent;
    tvis.hInsertAfter = insertAfter;
    tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
    if (imageIndex >= 0) {
        tvis.item.mask |= TVIF_IMAGE;
        tvis.item.iImage = imageIndex;
    }
    if (selectedImageIndex >= 0) {
        tvis.item.mask |= TVIF_SELECTEDIMAGE;
        tvis.item.iSelectedImage = selectedImageIndex;
    }
    tvis.item.pszText = (LPWSTR)text.c_str();
    tvis.item.lParam = param;
    
    return TreeView_InsertItem(m_hwnd, &tvis);
}

bool TreeView::deleteItem(TreeItem item) {
    if (!m_hwnd || !item) {
        return false;
    }
    
    return TreeView_DeleteItem(m_hwnd, item) != 0;
}

void TreeView::clear() {
    if (m_hwnd) {
        TreeView_DeleteAllItems(m_hwnd);
    }
}

bool TreeView::selectItem(TreeItem item) {
    if (!m_hwnd || !item) {
        return false;
    }
    
    return TreeView_SelectItem(m_hwnd, item) != 0;
}

TreeView::TreeItem TreeView::getSelectedItem() const {
    if (!m_hwnd) {
        return NULL;
    }
    
    return TreeView_GetSelection(m_hwnd);
}

bool TreeView::expandItem(TreeItem item, bool expand) {
    if (!m_hwnd || !item) {
        return false;
    }
    
    return TreeView_Expand(m_hwnd, item, expand ? TVE_EXPAND : TVE_COLLAPSE) != 0;
}

bool TreeView::setItemText(TreeItem item, const std::wstring& text) {
    if (!m_hwnd || !item) {
        return false;
    }
    
    TV_ITEM tvi = {0};
    tvi.mask = TVIF_TEXT;
    tvi.hItem = item;
    tvi.pszText = (LPWSTR)text.c_str();
    
    return TreeView_SetItem(m_hwnd, &tvi) != 0;
}

std::wstring TreeView::getItemText(TreeItem item) const {
    if (!m_hwnd || !item) {
        return L"";
    }
    
    std::wstring text;
    text.resize(256); // Zakładamy maksymalną długość tekstu
    
    TV_ITEM tvi = {0};
    tvi.mask = TVIF_TEXT;
    tvi.hItem = item;
    tvi.pszText = &text[0];
    tvi.cchTextMax = 256;
    
    TreeView_GetItem(m_hwnd, &tvi);
    
    // Usuwamy nadmiarowe znaki zerowe
    size_t length = wcslen(tvi.pszText);
    text.resize(length);
    
    return text;
}

bool TreeView::setItemData(TreeItem item, LPARAM param) {
    if (!m_hwnd || !item) {
        return false;
    }
    
    TV_ITEM tvi = {0};
    tvi.mask = TVIF_PARAM;
    tvi.hItem = item;
    tvi.lParam = param;
    
    return TreeView_SetItem(m_hwnd, &tvi) != 0;
}

LPARAM TreeView::getItemData(TreeItem item) const {
    if (!m_hwnd || !item) {
        return 0;
    }
    
    TV_ITEM tvi = {0};
    tvi.mask = TVIF_PARAM;
    tvi.hItem = item;
    
    TreeView_GetItem(m_hwnd, &tvi);
    
    return tvi.lParam;
}

TreeView::TreeItem TreeView::getParentItem(TreeItem item) const {
    if (!m_hwnd || !item) {
        return NULL;
    }
    
    return TreeView_GetParent(m_hwnd, item);
}

TreeView::TreeItem TreeView::getChildItem(TreeItem item) const {
    if (!m_hwnd) {
        return NULL;
    }
    
    if (item == NULL) {
        // Dla korzenia, pobierz pierwszy element
        return TreeView_GetRoot(m_hwnd);
    } else {
        return TreeView_GetChild(m_hwnd, item);
    }
}

TreeView::TreeItem TreeView::getNextSiblingItem(TreeItem item) const {
    if (!m_hwnd || !item) {
        return NULL;
    }
    
    return TreeView_GetNextSibling(m_hwnd, item);
}

void TreeView::setImageList(HIMAGELIST imageList) {
    if (m_hwnd) {
        TreeView_SetImageList(m_hwnd, imageList, TVSIL_NORMAL);
    }
}

void TreeView::setSelectionChangeHandler(std::function<void(TreeItem)> callback) {
    m_selectionChangeCallback = callback;
}

void TreeView::setItemDoubleClickHandler(std::function<void(TreeItem)> callback) {
    m_itemDoubleClickCallback = callback;
}

void TreeView::setItemExpandHandler(std::function<void(TreeItem, bool)> callback) {
    m_itemExpandCallback = callback;
}

bool TreeView::handleNotify(WPARAM wParam, LPARAM lParam) {
    NMHDR* pNMHDR = (NMHDR*)lParam;
    if (pNMHDR->hwndFrom == m_hwnd) {
        switch (pNMHDR->code) {
            case TVN_SELCHANGED: {
                NMTREEVIEW* pNMTV = (NMTREEVIEW*)lParam;
                if (m_selectionChangeCallback) {
                    m_selectionChangeCallback(pNMTV->itemNew.hItem);
                    return true;
                }
                break;
            }
            
            case NM_DBLCLK: {
                if (m_itemDoubleClickCallback) {
                    TreeItem item = getSelectedItem();
                    if (item) {
                        m_itemDoubleClickCallback(item);
                        return true;
                    }
                }
                break;
            }
            
            case TVN_ITEMEXPANDED: {
                NMTREEVIEW* pNMTV = (NMTREEVIEW*)lParam;
                if (m_itemExpandCallback) {
                    bool expanded = (pNMTV->action == TVE_EXPAND);
                    m_itemExpandCallback(pNMTV->itemNew.hItem, expanded);
                    return true;
                }
                break;
            }
        }
    }
    
    return false;
}

} // namespace WinAppLib