# Select — Lista rozwijana (ComboBox)

> `#include <UI/Select/Select.h>`

## Opis

Rozwijana lista wyboru (ComboBox). Obsługuje:
- Dodawanie elementów statycznie i dynamicznie
- Callback `onChange` przy zmianie wyboru
- Linkowanie z zewnętrznym wektorem (`link()`)
- Tekst w UTF-8 z automatyczną konwersją na Unicode

## Konstruktor

```cpp
Select(int x, int y, int width, int height, const char* text,
       std::function<void(Select*)> onChange);
```

| Parametr | Typ | Opis |
|----------|-----|------|
| `x`, `y` | `int` | Pozycja |
| `width`, `height` | `int` | Rozmiar (height = wysokość pola edycji) |
| `text` | `const char*` | Tekst początkowy (UTF-8) |
| `onChange` | `function<void(Select*)>` | Callback wywoływany przy zmianie wyboru |

## Metody

| Metoda | Zwraca | Opis |
|--------|--------|------|
| `create(HWND parent)` | `void` | Tworzy kontrolkę ComboBox |
| `addItem(const char* item)` | `void` | Dodaje element do listy |
| `setText(const char* text)` | `void` | Ustawia wybrany tekst |
| `clear()` | `void` | Czyści listę elementów |
| `link(const vector<string>* items)` | `void` | Linkuje z zewnętrznym wektorem |
| `updateItems()` | `void` | Aktualizuje listę z powiązanego wektora |
| `handleSelection()` | `void` | Obsługuje zmianę wyboru (wewnętrzne) |
| `getText()` | `const char*` | Tekst aktualnie wybranego elementu |
| `getHandle()` | `HWND` | Uchwyt kontrolki |
| `getId()` | `int` | Unikalny ID (auto od 3000) |

## Przykłady

### Lista z elementami statycznymi

```cpp
Select* sel = new Select(20, 50, 200, 25, "Wybierz port",
    [](Select* s) {
        const char* selected = s->getText();
        // Reakcja na wybór
    }
);
sel->addItem("COM1");
sel->addItem("COM3");
sel->addItem("COM5");
window->add(sel);
```

### Linkowanie z zewnętrznym wektorem

```cpp
Serial serial;
serial.init();

Select* selPort = new Select(20, 50, 200, 25, "Porty COM",
    [&serial](Select* s) {
        serial.setPort(s->getText());
    }
);
selPort->link(&serial.getAvailablePorts());
window->add(selPort);

// Gdy lista portów się zmieni:
serial.updateComPorts();
selPort->updateItems();   // Odświeża kontrolkę danymi z linked wektora
```

### Programowe ustawienie wyboru

```cpp
sel->setText("COM3");  // Zaznacza element "COM3" na liście
```

## Uwagi

- ID zaczynają się od **3000**
- `height` dotyczy pola edycji. Rozwijana lista jest automatycznie wyższa (+150 px)
- `link()` nie kopiuje danych, trzyma wskaźnik na wektor — wektor musi żyć dłużej niż Select
- `clear()` zeruje listę i `selectedIndex`
