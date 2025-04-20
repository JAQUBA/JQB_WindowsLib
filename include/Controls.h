#ifndef CONTROLS_H
#define CONTROLS_H

#include <windows.h>
#include <commctrl.h>
#include <string>
#include <map>
#include <functional>
#include <vector>

// Upewniamy się, że mamy dostęp do nowoczesnych kontrolek
#pragma comment(lib, "comctl32.lib")

// Definiujemy makro ListView_SetItemText jeśli nie zostało zdefiniowane
#ifndef ListView_SetItemText
#define ListView_SetItemText(hwndLV, i, iSubItem, pszText) \
    { LVITEM lvi; lvi.iSubItem = iSubItem; lvi.pszText = pszText; \
      SendMessage(hwndLV, LVM_SETITEMTEXT, (WPARAM)(i), (LPARAM)&lvi); }
#endif

// Definiujemy makro ListView_SetItemState jeśli nie zostało zdefiniowane
#ifndef ListView_SetItemState
#define ListView_SetItemState(hwndLV, i, data, mask) \
    { LVITEM lvi; lvi.state = data; lvi.stateMask = mask; \
      SendMessage(hwndLV, LVM_SETITEMSTATE, (WPARAM)(i), (LPARAM)&lvi); }
#endif

namespace WinAppLib {

/**
 * @brief Klasa bazowa dla kontrolek UI
 */
class Control {
public:
    /**
     * @brief Konstruktor
     * 
     * @param parent Uchwyt do okna rodzica
     * @param id Identyfikator kontrolki
     */
    Control(HWND parent, int id);
    
    /**
     * @brief Wirtualny destruktor
     */
    virtual ~Control();
    
    /**
     * @brief Pobiera uchwyt kontrolki
     * 
     * @return Uchwyt kontrolki
     */
    HWND getHandle() const { return m_hwnd; }
    
    /**
     * @brief Pobiera identyfikator kontrolki
     * 
     * @return Identyfikator kontrolki
     */
    int getId() const { return m_id; }
    
    /**
     * @brief Ustawia tekst kontrolki
     * 
     * @param text Tekst do ustawienia
     */
    void setText(const std::wstring& text);
    
    /**
     * @brief Pobiera tekst kontrolki
     * 
     * @return Tekst kontrolki
     */
    std::wstring getText() const;
    
    /**
     * @brief Włącza lub wyłącza kontrolkę
     * 
     * @param enable True dla włączenia, false dla wyłączenia
     */
    void enable(bool enable = true);
    
    /**
     * @brief Sprawdza czy kontrolka jest włączona
     * 
     * @return True jeśli kontrolka jest włączona
     */
    bool isEnabled() const;
    
    /**
     * @brief Pokazuje lub ukrywa kontrolkę
     * 
     * @param show True dla pokazania, false dla ukrycia
     */
    void show(bool show = true);
    
    /**
     * @brief Sprawdza czy kontrolka jest widoczna
     * 
     * @return True jeśli kontrolka jest widoczna
     */
    bool isVisible() const;

protected:
    HWND m_hwnd;    // Uchwyt kontrolki
    HWND m_parent;  // Uchwyt okna rodzica
    int m_id;       // Identyfikator kontrolki
};

/**
 * @brief Klasa przycisków
 */
class Button : public Control {
public:
    /**
     * @brief Konstruktor dla istniejącej kontrolki
     */
    Button(HWND parent, int id);
    
    /**
     * @brief Konstruktor tworzący nową kontrolkę
     */
    Button(HWND parent, int id, const std::wstring& text, int x, int y, int width, int height, DWORD style = 0);
    
    /**
     * @brief Destruktor
     */
    virtual ~Button();
    
    /**
     * @brief Rejestruje obsługę zdarzenia kliknięcia
     * 
     * @param callback Funkcja wywoływana przy kliknięciu
     */
    void setClickHandler(std::function<void()> callback);
    
    /**
     * @brief Obsługuje komunikat WM_COMMAND
     * 
     * @param wParam Parametr wParam
     * @param lParam Parametr lParam
     * @return True jeśli obsłużono zdarzenie
     */
    bool handleCommand(WPARAM wParam, LPARAM lParam);

private:
    std::function<void()> m_clickCallback;
};

/**
 * @brief Klasa pól tekstowych
 */
class TextBox : public Control {
public:
    /**
     * @brief Konstruktor dla istniejącej kontrolki
     */
    TextBox(HWND parent, int id);
    
    /**
     * @brief Konstruktor tworzący nową kontrolkę
     */
    TextBox(HWND parent, int id, const std::wstring& text, int x, int y, int width, int height, DWORD style = 0);
    
    /**
     * @brief Destruktor
     */
    virtual ~TextBox();
    
    /**
     * @brief Ustawia tekst pola
     * 
     * @param text Tekst do ustawienia
     */
    void setText(const std::wstring& text);
    
    /**
     * @brief Dodaje tekst na końcu pola
     * 
     * @param text Tekst do dodania
     */
    void appendText(const std::wstring& text);
    
    /**
     * @brief Czyści zawartość pola
     */
    void clear();
    
    /**
     * @brief Zaznacza wszystkie znaki
     */
    void selectAll();
    
    /**
     * @brief Przewija do końca
     */
    void scrollToEnd();
    
    /**
     * @brief Blokuje lub odblokowuje edycję
     * 
     * @param readOnly True dla zablokowania edycji
     */
    void setReadOnly(bool readOnly);
    
    /**
     * @brief Rejestruje obsługę zmiany tekstu
     * 
     * @param callback Funkcja wywoływana przy zmianie tekstu
     */
    void setChangeHandler(std::function<void(const std::wstring&)> callback);
    
    /**
     * @brief Obsługuje komunikat WM_COMMAND
     * 
     * @param wParam Parametr wParam
     * @param lParam Parametr lParam
     * @return True jeśli obsłużono zdarzenie
     */
    bool handleCommand(WPARAM wParam, LPARAM lParam);

private:
    std::function<void(const std::wstring&)> m_changeCallback;
};

/**
 * @brief Klasa pól kombi (ComboBox)
 */
class ComboBox : public Control {
public:
    /**
     * @brief Konstruktor dla istniejącej kontrolki
     */
    ComboBox(HWND parent, int id);
    
    /**
     * @brief Konstruktor tworzący nową kontrolkę
     */
    ComboBox(HWND parent, int id, int x, int y, int width, int height, DWORD style = 0);
    
    /**
     * @brief Destruktor
     */
    virtual ~ComboBox();
    
    /**
     * @brief Dodaje element do listy
     * 
     * @param text Tekst elementu
     * @param data Dane powiązane z elementem (opcjonalnie)
     * @return Indeks dodanego elementu
     */
    int addItem(const std::wstring& text, LPARAM data = 0);
    
    /**
     * @brief Wstawia element do listy
     * 
     * @param index Indeks, na którym należy wstawić element
     * @param text Tekst elementu
     * @param data Dane powiązane z elementem (opcjonalnie)
     * @return Indeks wstawionego elementu
     */
    int insertItem(int index, const std::wstring& text, LPARAM data = 0);
    
    /**
     * @brief Usuwa element z listy
     * 
     * @param index Indeks elementu do usunięcia
     * @return True jeśli udało się usunąć element
     */
    bool removeItem(int index);
    
    /**
     * @brief Czyści listę
     */
    void clear();
    
    /**
     * @brief Wybiera element z listy
     * 
     * @param index Indeks elementu do wybrania
     * @return True jeśli udało się wybrać element
     */
    bool selectItem(int index);
    
    /**
     * @brief Pobiera indeks wybranego elementu
     * 
     * @return Indeks wybranego elementu lub CB_ERR
     */
    int getSelectedIndex() const;
    
    /**
     * @brief Pobiera tekst wybranego elementu
     * 
     * @return Tekst wybranego elementu
     */
    std::wstring getSelectedText() const;
    
    /**
     * @brief Pobiera dane powiązane z wybranym elementem
     * 
     * @return Dane powiązane z wybranym elementem lub 0
     */
    LPARAM getSelectedData() const;
    
    /**
     * @brief Pobiera liczbę elementów w liście
     * 
     * @return Liczba elementów
     */
    int getCount() const;
    
    /**
     * @brief Pobiera tekst elementu o podanym indeksie
     * 
     * @param index Indeks elementu
     * @return Tekst elementu
     */
    std::wstring getItemText(int index) const;
    
    /**
     * @brief Pobiera dane powiązane z elementem o podanym indeksie
     * 
     * @param index Indeks elementu
     * @return Dane powiązane z elementem
     */
    LPARAM getItemData(int index) const;
    
    /**
     * @brief Rejestruje obsługę wyboru elementu
     * 
     * @param callback Funkcja wywoływana przy wyborze elementu
     */
    void setSelectionChangeHandler(std::function<void(int)> callback);
    
    /**
     * @brief Obsługuje komunikat WM_COMMAND
     * 
     * @param wParam Parametr wParam
     * @param lParam Parametr lParam
     * @return True jeśli obsłużono zdarzenie
     */
    bool handleCommand(WPARAM wParam, LPARAM lParam);

private:
    std::function<void(int)> m_selectionChangeCallback;
};

/**
 * @brief Klasa etykiet (Label)
 */
class Label : public Control {
public:
    /**
     * @brief Konstruktor dla istniejącej kontrolki
     */
    Label(HWND parent, int id);
    
    /**
     * @brief Konstruktor tworzący nową kontrolkę
     */
    Label(HWND parent, int id, const std::wstring& text, int x, int y, int width, int height, DWORD style = 0);
    
    /**
     * @brief Destruktor
     */
    virtual ~Label();
    
    /**
     * @brief Ustawia wyrównanie tekstu
     * 
     * @param align Wyrównanie tekstu (SS_LEFT, SS_CENTER, SS_RIGHT)
     */
    void setTextAlign(DWORD align);
};

/**
 * @brief Klasa pola kontrolnego (CheckBox)
 */
class CheckBox : public Control {
public:
    /**
     * @brief Konstruktor dla istniejącej kontrolki
     */
    CheckBox(HWND parent, int id);
    
    /**
     * @brief Konstruktor tworzący nową kontrolkę
     */
    CheckBox(HWND parent, int id, const std::wstring& text, int x, int y, int width, int height, DWORD style = 0);
    
    /**
     * @brief Destruktor
     */
    virtual ~CheckBox();
    
    /**
     * @brief Ustawia stan pola
     * 
     * @param checked True dla zaznaczenia, false dla odznaczenia
     */
    void setChecked(bool checked);
    
    /**
     * @brief Sprawdza stan pola
     * 
     * @return True jeśli pole jest zaznaczone
     */
    bool isChecked() const;
    
    /**
     * @brief Rejestruje obsługę zmiany stanu
     * 
     * @param callback Funkcja wywoływana przy zmianie stanu
     */
    void setStateChangeHandler(std::function<void(bool)> callback);
    
    /**
     * @brief Obsługuje komunikat WM_COMMAND
     * 
     * @param wParam Parametr wParam
     * @param lParam Parametr lParam
     * @return True jeśli obsłużono zdarzenie
     */
    bool handleCommand(WPARAM wParam, LPARAM lParam);

private:
    std::function<void(bool)> m_stateChangeCallback;
};

/**
 * @brief Klasa przycisków radiowych (RadioButton)
 */
class RadioButton : public Control {
public:
    /**
     * @brief Konstruktor dla istniejącej kontrolki
     */
    RadioButton(HWND parent, int id);
    
    /**
     * @brief Konstruktor tworzący nową kontrolkę
     */
    RadioButton(HWND parent, int id, const std::wstring& text, int x, int y, int width, int height, DWORD style = 0);
    
    /**
     * @brief Destruktor
     */
    virtual ~RadioButton();
    
    /**
     * @brief Ustawia stan przycisku radiowego
     * 
     * @param checked True dla zaznaczenia, false dla odznaczenia
     */
    void setChecked(bool checked);
    
    /**
     * @brief Sprawdza stan przycisku radiowego
     * 
     * @return True jeśli przycisk jest zaznaczony
     */
    bool isChecked() const;
    
    /**
     * @brief Rejestruje obsługę zmiany stanu
     * 
     * @param callback Funkcja wywoływana przy zmianie stanu
     */
    void setStateChangeHandler(std::function<void()> callback);
    
    /**
     * @brief Obsługuje komunikat WM_COMMAND
     * 
     * @param wParam Parametr wParam
     * @param lParam Parametr lParam
     * @return True jeśli obsłużono zdarzenie
     */
    bool handleCommand(WPARAM wParam, LPARAM lParam);

private:
    std::function<void()> m_stateChangeCallback;
};

/**
 * @brief Klasa do grupowania kontrolek (GroupBox)
 */
class GroupBox : public Control {
public:
    /**
     * @brief Konstruktor dla istniejącej kontrolki
     */
    GroupBox(HWND parent, int id);
    
    /**
     * @brief Konstruktor tworzący nową kontrolkę
     */
    GroupBox(HWND parent, int id, const std::wstring& text, int x, int y, int width, int height, DWORD style = 0);
    
    /**
     * @brief Destruktor
     */
    virtual ~GroupBox();
};

/**
 * @brief Klasa paska postępu (ProgressBar)
 */
class ProgressBar : public Control {
public:
    /**
     * @brief Konstruktor dla istniejącej kontrolki
     */
    ProgressBar(HWND parent, int id);
    
    /**
     * @brief Konstruktor tworzący nową kontrolkę
     */
    ProgressBar(HWND parent, int id, int x, int y, int width, int height, DWORD style = 0);
    
    /**
     * @brief Destruktor
     */
    virtual ~ProgressBar();
    
    /**
     * @brief Ustawia zakres paska postępu
     * 
     * @param minValue Minimalna wartość
     * @param maxValue Maksymalna wartość
     */
    void setRange(int minValue, int maxValue);
    
    /**
     * @brief Pobiera minimalną wartość zakresu
     * 
     * @return Minimalna wartość
     */
    int getMinRange() const;
    
    /**
     * @brief Pobiera maksymalną wartość zakresu
     * 
     * @return Maksymalna wartość
     */
    int getMaxRange() const;
    
    /**
     * @brief Ustawia aktualną pozycję paska postępu
     * 
     * @param pos Nowa pozycja
     * @return Poprzednia pozycja
     */
    int setPosition(int pos);
    
    /**
     * @brief Pobiera aktualną pozycję paska postępu
     * 
     * @return Aktualna pozycja
     */
    int getPosition() const;
    
    /**
     * @brief Zwiększa pozycję paska postępu o określoną wartość
     * 
     * @param increment Wartość zwiększenia
     * @return Nowa pozycja
     */
    int deltaPosition(int increment);
    
    /**
     * @brief Ustawia styl paska postępu (normalny, kolorowy)
     * 
     * @param state Nowy stan (PBS_NORMAL, PBS_MARQUEE)
     */
    void setState(int state);
    
    /**
     * @brief Ustawia kolor paska postępu
     * 
     * @param color Nowy kolor
     */
    void setBarColor(COLORREF color);
    
    /**
     * @brief Ustawia kolor tła paska postępu
     * 
     * @param color Nowy kolor tła
     */
    void setBkColor(COLORREF color);
};

/**
 * @brief Klasa suwaka (TrackBar/Slider)
 */
class TrackBar : public Control {
public:
    /**
     * @brief Konstruktor dla istniejącej kontrolki
     */
    TrackBar(HWND parent, int id);
    
    /**
     * @brief Konstruktor tworzący nową kontrolkę
     */
    TrackBar(HWND parent, int id, int x, int y, int width, int height, DWORD style = 0);
    
    /**
     * @brief Destruktor
     */
    virtual ~TrackBar();
    
    /**
     * @brief Ustawia zakres suwaka
     * 
     * @param minValue Minimalna wartość
     * @param maxValue Maksymalna wartość
     */
    void setRange(int minValue, int maxValue);
    
    /**
     * @brief Pobiera minimalną wartość zakresu
     * 
     * @return Minimalna wartość
     */
    int getMinRange() const;
    
    /**
     * @brief Pobiera maksymalną wartość zakresu
     * 
     * @return Maksymalna wartość
     */
    int getMaxRange() const;
    
    /**
     * @brief Ustawia aktualną pozycję suwaka
     * 
     * @param pos Nowa pozycja
     * @return Poprzednia pozycja
     */
    int setPosition(int pos);
    
    /**
     * @brief Pobiera aktualną pozycję suwaka
     * 
     * @return Aktualna pozycja
     */
    int getPosition() const;
    
    /**
     * @brief Ustawia rozmiar kroku suwaka
     * 
     * @param lineSize Rozmiar małej zmiany
     * @param pageSize Rozmiar dużej zmiany
     */
    void setTickFrequency(int frequency);
    
    /**
     * @brief Ustawia widoczność znaczników podziałki
     * 
     * @param show True aby pokazać znaczniki
     */
    void showTicks(bool show);
    
    /**
     * @brief Rejestruje obsługę zmiany pozycji
     * 
     * @param callback Funkcja wywoływana przy zmianie pozycji
     */
    void setPositionChangeHandler(std::function<void(int)> callback);
    
    /**
     * @brief Obsługuje komunikaty kontrolki
     * 
     * @param wParam Parametr wParam
     * @param lParam Parametr lParam
     * @return True jeśli obsłużono zdarzenie
     */
    bool handleNotify(WPARAM wParam, LPARAM lParam);

private:
    std::function<void(int)> m_positionChangeCallback;
};

/**
 * @brief Klasa paska statusu (StatusBar)
 */
class StatusBar : public Control {
public:
    /**
     * @brief Konstruktor dla istniejącej kontrolki
     */
    StatusBar(HWND parent, int id);
    
    /**
     * @brief Konstruktor tworzący nową kontrolkę
     */
    StatusBar(HWND parent, int id, DWORD style = SBARS_SIZEGRIP);
    
    /**
     * @brief Destruktor
     */
    virtual ~StatusBar();
    
    /**
     * @brief Ustawia tekst w określonej części paska statusu
     * 
     * @param partIndex Indeks części (0 dla domyślnej)
     * @param text Tekst do ustawienia
     */
    void setText(int partIndex, const std::wstring& text);
    
    /**
     * @brief Ustawia tekst w domyślnej (pierwszej) części paska statusu
     * 
     * @param text Tekst do ustawienia
     */
    void setText(const std::wstring& text);
    
    /**
     * @brief Pobiera tekst z określonej części paska statusu
     * 
     * @param partIndex Indeks części (0 dla domyślnej)
     * @return Tekst z określonej części
     */
    std::wstring getText(int partIndex = 0) const;
    
    /**
     * @brief Ustawia ikony dla paska statusu
     * 
     * @param icons Tablica uchwytów ikon
     * @param count Liczba ikon
     */
    void setIcons(HICON* icons, int count);
    
    /**
     * @brief Ustawia podział paska statusu na części
     * 
     * @param parts Tablica szerokości części
     * @param count Liczba części
     */
    void setParts(int count, const int* widths);
    
    /**
     * @brief Pobiera liczbę części paska statusu
     * 
     * @return Liczba części
     */
    int getPartsCount() const;
    
    /**
     * @brief Dostosowuje pasek statusu do rozmiaru okna rodzica
     */
    void autoSize();
};

/**
 * @brief Klasa kontrolki z zakładkami (TabControl)
 */
class TabControl : public Control {
public:
    /**
     * @brief Konstruktor dla istniejącej kontrolki
     */
    TabControl(HWND parent, int id);
    
    /**
     * @brief Konstruktor tworzący nową kontrolkę
     */
    TabControl(HWND parent, int id, int x, int y, int width, int height, DWORD style = 0);
    
    /**
     * @brief Destruktor
     */
    virtual ~TabControl();
    
    /**
     * @brief Dodaje nową zakładkę
     * 
     * @param text Tytuł zakładki
     * @param param Parametr użytkownika
     * @param imageIndex Indeks ikony w liście obrazów (lub -1)
     * @return Indeks dodanej zakładki lub -1 w przypadku błędu
     */
    int addTab(const std::wstring& text, LPARAM param = 0, int imageIndex = -1);
    
    /**
     * @brief Usuwa zakładkę
     * 
     * @param index Indeks zakładki do usunięcia
     * @return True jeśli udało się usunąć zakładkę
     */
    bool deleteTab(int index);
    
    /**
     * @brief Wybiera aktywną zakładkę
     * 
     * @param index Indeks zakładki do wybrania
     * @return True jeśli udało się wybrać zakładkę
     */
    bool selectTab(int index);
    
    /**
     * @brief Pobiera indeks aktywnej zakładki
     * 
     * @return Indeks aktywnej zakładki lub -1
     */
    int getSelectedTabIndex() const;
    
    /**
     * @brief Pobiera parametr użytkownika dla zakładki
     * 
     * @param index Indeks zakładki
     * @return Parametr użytkownika
     */
    LPARAM getTabParam(int index) const;
    
    /**
     * @brief Ustawia parametr użytkownika dla zakładki
     * 
     * @param index Indeks zakładki
     * @param param Nowy parametr użytkownika
     */
    void setTabParam(int index, LPARAM param);
    
    /**
     * @brief Pobiera tekst zakładki
     * 
     * @param index Indeks zakładki
     * @return Tekst zakładki
     */
    std::wstring getTabText(int index) const;
    
    /**
     * @brief Ustawia tekst zakładki
     * 
     * @param index Indeks zakładki
     * @param text Nowy tekst zakładki
     */
    void setTabText(int index, const std::wstring& text);
    
    /**
     * @brief Pobiera liczbę zakładek
     * 
     * @return Liczba zakładek
     */
    int getTabCount() const;
    
    /**
     * @brief Ustawia listę ikon dla zakładek
     * 
     * @param imageList Uchwyt do listy ikon
     */
    void setImageList(HIMAGELIST imageList);
    
    /**
     * @brief Rejestruje obsługę zmiany zakładki
     * 
     * @param callback Funkcja wywoływana przy zmianie zakładki
     */
    void setTabChangeHandler(std::function<void(int)> callback);
    
    /**
     * @brief Obsługuje komunikaty kontrolki
     * 
     * @param wParam Parametr wParam
     * @param lParam Parametr lParam
     * @return True jeśli obsłużono zdarzenie
     */
    bool handleNotify(WPARAM wParam, LPARAM lParam);
    
    /**
     * @brief Oblicza prostokąt zawartości dla aktywnej zakładki
     * 
     * @param rect Referencja do struktury RECT, która otrzyma współrzędne
     */
    void getClientRect(RECT& rect) const;

private:
    std::function<void(int)> m_tabChangeCallback;
};

/**
 * @brief Klasa kontrolki listy (ListView)
 */
class ListView : public Control {
public:
    /**
     * @brief Konstruktor dla istniejącej kontrolki
     */
    ListView(HWND parent, int id);
    
    /**
     * @brief Konstruktor tworzący nową kontrolkę
     */
    ListView(HWND parent, int id, int x, int y, int width, int height, DWORD style = LVS_REPORT | LVS_SHOWSELALWAYS);
    
    /**
     * @brief Destruktor
     */
    virtual ~ListView();
    
    /**
     * @brief Dodaje kolumnę do listy
     * 
     * @param index Indeks kolumny
     * @param text Tytuł kolumny
     * @param width Szerokość kolumny
     * @param format Format wyświetlania (LVCFMT_xxx)
     * @return Indeks dodanej kolumny lub -1
     */
    int addColumn(int index, const std::wstring& text, int width, int format = LVCFMT_LEFT);
    
    /**
     * @brief Usuwa kolumnę z listy
     * 
     * @param index Indeks kolumny do usunięcia
     * @return True jeśli udało się usunąć kolumnę
     */
    bool deleteColumn(int index);
    
    /**
     * @brief Dodaje element do listy
     * 
     * @param text Tekst pierwszej kolumny elementu
     * @param imageIndex Indeks ikony w liście obrazów (lub -1)
     * @return Indeks dodanego elementu lub -1
     */
    int addItem(const std::wstring& text, int imageIndex = -1);
    
    /**
     * @brief Ustawia tekst dla podanego elementu i kolumny
     * 
     * @param itemIndex Indeks elementu
     * @param subItemIndex Indeks podpozycji (kolumny)
     * @param text Tekst do ustawienia
     * @return True jeśli udało się ustawić tekst
     */
    bool setItemText(int itemIndex, int subItemIndex, const std::wstring& text);
    
    /**
     * @brief Pobiera tekst dla podanego elementu i kolumny
     * 
     * @param itemIndex Indeks elementu
     * @param subItemIndex Indeks podpozycji (kolumny)
     * @return Tekst elementu
     */
    std::wstring getItemText(int itemIndex, int subItemIndex) const;
    
    /**
     * @brief Ustawia dane użytkownika dla elementu
     * 
     * @param index Indeks elementu
     * @param data Dane użytkownika
     * @return True jeśli udało się ustawić dane
     */
    bool setItemData(int index, LPARAM data);
    
    /**
     * @brief Pobiera dane użytkownika dla elementu
     * 
     * @param index Indeks elementu
     * @return Dane użytkownika
     */
    LPARAM getItemData(int index) const;
    
    /**
     * @brief Usuwa element z listy
     * 
     * @param index Indeks elementu do usunięcia
     * @return True jeśli udało się usunąć element
     */
    bool deleteItem(int index);
    
    /**
     * @brief Usuwa wszystkie elementy z listy
     */
    void clear();
    
    /**
     * @brief Wybiera element listy
     * 
     * @param index Indeks elementu do wybrania
     * @param deselectOthers True aby odznaczyć inne elementy
     * @return True jeśli udało się wybrać element
     */
    bool selectItem(int index, bool deselectOthers = true);
    
    /**
     * @brief Sprawdza czy element jest wybrany
     * 
     * @param index Indeks elementu do sprawdzenia
     * @return True jeśli element jest wybrany
     */
    bool isItemSelected(int index) const;
    
    /**
     * @brief Pobiera indeks pierwszego wybranego elementu
     * 
     * @return Indeks wybranego elementu lub -1
     */
    int getSelectedIndex() const;
    
    /**
     * @brief Pobiera wszystkie wybrane indeksy
     * 
     * @return Wektor z indeksami wybranych elementów
     */
    std::vector<int> getSelectedIndices() const;
    
    /**
     * @brief Pobiera liczbę elementów w liście
     * 
     * @return Liczba elementów
     */
    int getItemCount() const;
    
    /**
     * @brief Ustawia listę ikon dla elementów
     * 
     * @param imageList Uchwyt do listy ikon
     * @param type Typ listy ikon (LVSIL_NORMAL, LVSIL_SMALL, LVSIL_STATE)
     */
    void setImageList(HIMAGELIST imageList, int type);
    
    /**
     * @brief Włącza lub wyłącza pełne zaznaczanie wierszy
     * 
     * @param enable True aby włączyć pełne zaznaczanie wierszy
     */
    void enableFullRowSelect(bool enable = true);
    
    /**
     * @brief Włącza lub wyłącza linie siatki
     * 
     * @param enable True aby wyświetlać linie siatki
     */
    void enableGridLines(bool enable = true);
    
    /**
     * @brief Rejestruje obsługę zdarzenia kliknięcia elementu
     * 
     * @param callback Funkcja wywoływana przy kliknięciu
     */
    void setItemClickHandler(std::function<void(int)> callback);
    
    /**
     * @brief Rejestruje obsługę zdarzenia podwójnego kliknięcia elementu
     * 
     * @param callback Funkcja wywoływana przy podwójnym kliknięciu
     */
    void setItemDoubleClickHandler(std::function<void(int)> callback);
    
    /**
     * @brief Rejestruje obsługę zdarzenia zmiany wyboru
     * 
     * @param callback Funkcja wywoływana przy zmianie wyboru
     */
    void setSelectionChangeHandler(std::function<void()> callback);
    
    /**
     * @brief Obsługuje komunikaty kontrolki
     * 
     * @param wParam Parametr wParam
     * @param lParam Parametr lParam
     * @return True jeśli obsłużono zdarzenie
     */
    bool handleNotify(WPARAM wParam, LPARAM lParam);

private:
    std::function<void(int)> m_itemClickCallback;
    std::function<void(int)> m_itemDoubleClickCallback;
    std::function<void()> m_selectionChangeCallback;
};

/**
 * @brief Klasa kontrolki drzewa (TreeView)
 */
class TreeView : public Control {
public:
    /**
     * @brief Uchwyt elementu drzewa
     */
    using TreeItem = HTREEITEM;
    
    /**
     * @brief Konstruktor dla istniejącej kontrolki
     */
    TreeView(HWND parent, int id);
    
    /**
     * @brief Konstruktor tworzący nową kontrolkę
     */
    TreeView(HWND parent, int id, int x, int y, int width, int height, DWORD style = TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS);
    
    /**
     * @brief Destruktor
     */
    virtual ~TreeView();
    
    /**
     * @brief Dodaje węzeł do drzewa
     * 
     * @param text Tekst węzła
     * @param parent Węzeł nadrzędny lub NULL dla korzenia
     * @param insertAfter Węzeł, po którym wstawić, lub specjalna wartość
     * @param imageIndex Indeks ikony w liście obrazów (lub -1)
     * @param selectedImageIndex Indeks ikony w stanie wybranym (lub -1)
     * @param param Parametr użytkownika
     * @return Uchwyt dodanego węzła lub NULL
     */
    TreeItem addItem(const std::wstring& text, TreeItem parent = NULL, TreeItem insertAfter = TVI_LAST, 
                       int imageIndex = -1, int selectedImageIndex = -1, LPARAM param = 0);
    
    /**
     * @brief Usuwa węzeł z drzewa
     * 
     * @param item Węzeł do usunięcia
     * @return True jeśli udało się usunąć węzeł
     */
    bool deleteItem(TreeItem item);
    
    /**
     * @brief Usuwa wszystkie węzły z drzewa
     */
    void clear();
    
    /**
     * @brief Wybiera węzeł
     * 
     * @param item Węzeł do wybrania
     * @return True jeśli udało się wybrać węzeł
     */
    bool selectItem(TreeItem item);
    
    /**
     * @brief Pobiera wybrany węzeł
     * 
     * @return Uchwyt wybranego węzła lub NULL
     */
    TreeItem getSelectedItem() const;
    
    /**
     * @brief Rozwija lub zwija węzeł
     * 
     * @param item Węzeł do rozwinięcia/zwinięcia
     * @param expand True aby rozwinąć, false aby zwinąć
     * @return True jeśli operacja się powiodła
     */
    bool expandItem(TreeItem item, bool expand = true);
    
    /**
     * @brief Ustawia tekst węzła
     * 
     * @param item Węzeł
     * @param text Nowy tekst
     * @return True jeśli udało się ustawić tekst
     */
    bool setItemText(TreeItem item, const std::wstring& text);
    
    /**
     * @brief Pobiera tekst węzła
     * 
     * @param item Węzeł
     * @return Tekst węzła
     */
    std::wstring getItemText(TreeItem item) const;
    
    /**
     * @brief Ustawia parametr użytkownika dla węzła
     * 
     * @param item Węzeł
     * @param param Nowy parametr
     * @return True jeśli udało się ustawić parametr
     */
    bool setItemData(TreeItem item, LPARAM param);
    
    /**
     * @brief Pobiera parametr użytkownika dla węzła
     * 
     * @param item Węzeł
     * @return Parametr użytkownika
     */
    LPARAM getItemData(TreeItem item) const;
    
    /**
     * @brief Pobiera węzeł nadrzędny dla podanego węzła
     * 
     * @param item Węzeł
     * @return Węzeł nadrzędny lub NULL
     */
    TreeItem getParentItem(TreeItem item) const;
    
    /**
     * @brief Pobiera pierwszy potomek węzła
     * 
     * @param item Węzeł
     * @return Pierwszy potomek lub NULL
     */
    TreeItem getChildItem(TreeItem item) const;
    
    /**
     * @brief Pobiera następny węzeł na tym samym poziomie
     * 
     * @param item Węzeł
     * @return Następny węzeł lub NULL
     */
    TreeItem getNextSiblingItem(TreeItem item) const;
    
    /**
     * @brief Ustawia listę ikon dla węzłów
     * 
     * @param imageList Uchwyt do listy ikon
     */
    void setImageList(HIMAGELIST imageList);
    
    /**
     * @brief Rejestruje obsługę zdarzenia zmiany wyboru
     * 
     * @param callback Funkcja wywoływana przy zmianie wyboru
     */
    void setSelectionChangeHandler(std::function<void(TreeItem)> callback);
    
    /**
     * @brief Rejestruje obsługę zdarzenia podwójnego kliknięcia
     * 
     * @param callback Funkcja wywoływana przy podwójnym kliknięciu
     */
    void setItemDoubleClickHandler(std::function<void(TreeItem)> callback);
    
    /**
     * @brief Rejestruje obsługę zdarzenia rozwinięcia/zwinięcia węzła
     * 
     * @param callback Funkcja wywoływana przy rozwinięciu/zwinięciu węzła
     */
    void setItemExpandHandler(std::function<void(TreeItem, bool)> callback);
    
    /**
     * @brief Obsługuje komunikaty kontrolki
     * 
     * @param wParam Parametr wParam
     * @param lParam Parametr lParam
     * @return True jeśli obsłużono zdarzenie
     */
    bool handleNotify(WPARAM wParam, LPARAM lParam);

private:
    std::function<void(TreeItem)> m_selectionChangeCallback;
    std::function<void(TreeItem)> m_itemDoubleClickCallback;
    std::function<void(TreeItem, bool)> m_itemExpandCallback;
};

} // namespace WinAppLib

#endif // CONTROLS_H