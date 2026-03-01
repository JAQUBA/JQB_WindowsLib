Import("env")
import os
import subprocess
import sys
import platform as platform_module

# Lokalizacja pliku resources.rc
rc_file = os.path.join(env.subst("$PROJECT_DIR"), "resources.rc")

# Katalog dla skompilowanego pliku .res
build_dir = os.path.join(env.subst("$PROJECT_DIR"), ".pio", "build")

# Utwórz katalog build jeśli nie istnieje
if not os.path.exists(build_dir):
    os.makedirs(build_dir)

res_file = os.path.join(build_dir, "resources.res")

# Znajdź ścieżkę do MinGW - sprawdza wiele możliwych lokalizacji
def find_mingw_bin():
    """Znajdź ścieżkę do katalogu bin MinGW"""
    project_dir = env.subst("$PROJECT_DIR")
    
    # Wykryj architekturę
    is_64bit = platform_module.machine().endswith('64')
    
    # --- 1. PlatformIO packages directory (JQB_MinGW platform) ---
    pio_home = os.environ.get(
        "PLATFORMIO_HOME_DIR",
        os.path.join(os.path.expanduser("~"), ".platformio"),
    )
    pio_packages = os.path.join(pio_home, "packages")
    for mingw_name in ["toolchain-mingw64", "toolchain-mingw32"]:
        bin_path = os.path.join(pio_packages, mingw_name, "bin")
        windres_path = os.path.join(bin_path, "windres.exe")
        if os.path.exists(windres_path):
            return bin_path
    
    # --- 2. Project-local platform/packages directories ---
    possible_packages = [
        os.path.join(project_dir, "platform", "packages"),
        os.path.join(project_dir, "..", "platform", "packages"),
    ]
    
    # Nazwy folderów MinGW
    mingw_names = ["mingw64", "toolchain-mingw64"] if is_64bit else ["mingw32", "toolchain-mingw32"]
    
    for packages_dir in possible_packages:
        if not os.path.exists(packages_dir):
            continue
            
        # Sprawdź standardowe nazwy
        for mingw_name in mingw_names:
            bin_path = os.path.join(packages_dir, mingw_name, "bin")
            windres_path = os.path.join(bin_path, "windres.exe")
            if os.path.exists(windres_path):
                return bin_path
        
        # Fallback - szukaj dowolnego folderu z mingw w nazwie
        for item in os.listdir(packages_dir):
            item_path = os.path.join(packages_dir, item)
            if os.path.isdir(item_path) and "mingw" in item.lower():
                bin_path = os.path.join(item_path, "bin")
                windres_path = os.path.join(bin_path, "windres.exe")
                if os.path.exists(windres_path):
                    return bin_path
    
    return None

# Komenda do kompilacji pliku RC
if sys.platform == "win32":
    # Znajdź pełną ścieżkę do windres
    mingw_bin = find_mingw_bin()
    if mingw_bin:
        windres_path = os.path.join(mingw_bin, "windres.exe")
        # Dodaj MinGW bin do PATH dla innych narzędzi
        env.PrependENVPath("PATH", mingw_bin)
    else:
        windres_path = "windres"  # Fallback do PATH
    
    windres_cmd = [windres_path, rc_file, "-O", "coff", "-o", res_file]
    
    print("Kompilowanie plików zasobów: " + " ".join(windres_cmd))
    subprocess.call(windres_cmd)
    
    # Dodaj plik .res do procesu linkowania
    env.Append(LINKFLAGS=[res_file])
    
    # Dodaj definicje Unicode (wymagane przez WinAPI)
    print("Dodawanie definicji UNICODE")
    env.Append(CPPDEFINES=["UNICODE", "_UNICODE"])

    # Dodaj flagę C++17 (wymagane przez bibliotekę JQB)
    print("Dodawanie flagi C++17")
    env.Append(CXXFLAGS=["-std=c++17"])

    # Dodaj flagę subsystem,windows przenosząc ją z platformio.ini
    print("Dodawanie flagi subsystem,windows")
    env.Append(LINKFLAGS=["-Wl,-subsystem,windows"])
    
    # Dodaj flagi statycznego linkowania
    print("Dodawanie flag statycznego linkowania")
    env.Append(LINKFLAGS=["-static-libgcc", "-static-libstdc++", "-static"])
    
    # Dodaj wszystkie potrzebne biblioteki dla komponentów UI i funkcjonalności
    # Biblioteki IO (setupapi, bthprops, hid, gdiplus, shlwapi) są ładowane
    # dynamicznie przez LoadLibrary/GetProcAddress w odpowiednich modułach.
    print("Dodawanie bibliotek do procesu linkowania")
    libraries = [
        "gdi32",           # Grafika GDI (używane globalnie w UI)
        "comctl32",        # Kontrolki Windows (rejestracja klas)
    ]
    env.Append(LIBS=libraries)
    print(f"Dodano następujące biblioteki: {', '.join(libraries)}")