# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## État actuel du dépôt

Le MVP compile et produit un `.vpk` fonctionnel (`build/hello_vita.vpk`). Structure en place :
- `CMakeLists.txt` — build config
- `src/main.c` — boucle principale (affichage + lecture bouton START)
- `src/debugScreen.c` / `debugScreen.h` / `debugScreenFont.c` / `debugScreen_custom.h` — moteur d'affichage texte (récupéré tel quel depuis `vitasdk/samples/common`, ne pas réécrire à la main)
- `scripts/setup_env.sh` — configure les permissions + `~/.bashrc` dans la distro WSL (idempotent, à relancer si le dossier VitaSDK est recréé)
- `scripts/build.sh` — configure et compile le projet

Reste à faire (hors scope MVP strict, itérations futures) : `sce_sys/` (icône/LiveArea), test réel sur console via VitaShell/FTP.

`PRD.md` reste la référence pour le scope (MVP = affichage texte + lecture bouton + packaging `.vpk` ; hors scope = libvita2d, réseau, audio, sauvegarde, UI complexe).

## Nature du projet

Application homebrew PS Vita minimale, écrite en C avec le VitaSDK, dont le but est de valider la chaîne de compilation/packaging/déploiement de bout en bout (et non de livrer une app complète). Toute implémentation doit rester dans le périmètre MVP du PRD tant qu'il n'est pas explicitement étendu.

## Toolchain — installée sous WSL2/Ubuntu (pas MSYS2)

Choix fait pour ce projet : la toolchain VitaSDK tourne **dans une distribution WSL2 Ubuntu**, pas nativement sous Windows/MSYS2. Raison : le script d'installation officiel de VitaSDK (`vdpm`/`bootstrap-vitasdk.sh`) est écrit pour Linux, et WSL évite les problèmes d'encodage de chemins que MSYS2 peut avoir sur Windows.

- Distribution : `Ubuntu` (`wsl -d Ubuntu`), utilisateur `berete`.
- VitaSDK installé dans `/usr/local/vitasdk` (variable `VITASDK`), exports persistés dans `~/.bashrc` de la distro.
- Le dépôt Windows (`D:\Bérété\projets\psvita`) est accédé depuis WSL via `/mnt/d/Bérété/projets/psvita` — **testé et fonctionnel malgré l'accent**, contrairement à l'hypothèse initiale de risque d'encodage (celle-ci ne concernait que MSYS2 natif, pas WSL).
- Transfert vers la console via VitaShell + FTP (pas de câble/USB dans ce workflow).

### Piège connu : invoquer WSL depuis PowerShell

Passer des commandes multi-instructions à `wsl -d Ubuntu -- bash -c '...'` depuis PowerShell est fragile dès que la commande contient `$PATH`/`$VAR`, des pipes `|`, ou des guillemets imbriqués — le résultat peut être une sortie totalement vide ou tronquée sans message d'erreur clair (bug d'interop constaté empiriquement, cause exacte non identifiée). **Solution qui fonctionne de façon fiable** : écrire un script `.sh` dans `scripts/` puis l'exécuter via `wsl -d Ubuntu -u <user> -- bash /mnt/d/Bérété/projets/psvita/scripts/xxx.sh`. Éviter les commandes bash inline complexes passées en argument depuis PowerShell.

## Commandes de build

Depuis PowerShell (ou tout shell Windows), en invoquant WSL :

```bash
wsl -d Ubuntu -u berete -- bash /mnt/d/Bérété/projets/psvita/scripts/build.sh
```

Équivalent manuel, exécuté **depuis un shell Ubuntu** (WSL) avec `VITASDK`/`PATH` déjà exportés (via `~/.bashrc`, cf. `scripts/setup_env.sh`) :

```bash
cmake -Bbuild -S. -DCMAKE_TOOLCHAIN_FILE="$VITASDK/share/vita.toolchain.cmake" -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Le `.vpk` est généré dans `build/` (`build/hello_vita.vpk`) et s'installe sur la console via VitaShell (copie sur la carte SD puis installation locale) ou via FTP.

## Points d'attention spécifiques à cet environnement

- Développement fait sous Windows 11, mais la compilation elle-même se fait **dans WSL2 Ubuntu**, pas en PowerShell/Git Bash natif — ne pas essayer d'appeler `arm-vita-eabi-gcc`/`cmake` directement depuis Windows, ils n'y sont pas installés.
- `install-all.sh` de `vdpm` (installation de bibliothèques additionnelles type SDL2/libvita2d) n'a **pas** été exécuté et n'est pas nécessaire tant que le scope reste celui du MVP (`SceDisplay_stub` + `SceCtrl_stub` suffisent, fournis par le bootstrap de base).
- L'API GitHub non authentifiée est limitée à 60 requêtes/heure — le script `bootstrap-vitasdk.sh` de VitaSDK peut échouer avec `HTTP Error 403: rate limit exceeded` si ce quota est épuisé (arrive vite si on relance le script plusieurs fois). Attendre le reset (`curl https://api.github.com/rate_limit`) ou s'authentifier via `gh auth login` pour lever la limite à 5000/h.
