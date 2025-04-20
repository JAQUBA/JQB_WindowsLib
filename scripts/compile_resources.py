Import("env")
import os
import subprocess
import sys

# Lokalizacja pliku resources.rc
rc_file = os.path.join(env.subst("$PROJECT_DIR"), "resources.rc")

# Katalog dla skompilowanego pliku .res
build_dir = os.path.join(env.subst("$PROJECT_DIR"), ".pio", "build")

# Utwórz katalog build jeśli nie istnieje
if not os.path.exists(build_dir):
    os.makedirs(build_dir)

res_file = os.path.join(build_dir, "resources.res")

# Komenda do kompilacji pliku RC
if sys.platform == "win32":
    windres_cmd = ["windres", rc_file, "-O", "coff", "-o", res_file]
    
    print("Kompilowanie plików zasobów: " + " ".join(windres_cmd))
    subprocess.call(windres_cmd)
    
    # Dodaj plik .res do procesu linkowania
    env.Append(LINKFLAGS=[res_file])