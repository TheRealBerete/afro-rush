# PRD — Application Homebrew PS Vita (Projet basique)

## 1. Contexte
Développement d'une première application homebrew pour PS Vita afin de valider toute la chaîne de développement : environnement, compilation, packaging `.vpk`, transfert et exécution sur console jailbreakée. Objectif principal : apprentissage et validation technique, pas une app commerciale.

## 2. Objectif
Créer et exécuter avec succès une application `.vpk` simple sur une PS Vita jailbreakée (Henkaku/Enso ou h-encore), développée avec l'aide de Claude Code.

## 3. Périmètre (Scope)

### Inclus (MVP)
- Affichage d'un texte à l'écran (ex: "Hello World" ou message personnalisé)
- Lecture des boutons de base (ex: quitter avec le bouton Start/Select)
- Packaging en `.vpk` installable
- Icône et infos LiveArea basiques (nom de l'app, titre ID)

### Exclus (hors MVP, évolutions futures possibles)
- Rendu graphique avancé (sprites, textures, libvita2d)
- Réseau / HTTP
- Audio
- Sauvegarde de données
- Interface utilisateur complexe (menus, dialogues)

## 4. Prérequis techniques

| Élément | Détail |
|---|---|
| Console | PS Vita jailbreakée (déjà fait) |
| Toolchain | VitaSDK (`arm-vita-eabi-gcc`, `vita-toolchain`) |
| Build system | CMake |
| Langage | C (ou C++ si besoin) |
| Transfert | VitaShell + FTP |
| Assistance dev | Claude Code |

## 5. Fonctionnalités détaillées

1. **Affichage écran**
   - Utiliser `psvDebugScreenPrintf` ou équivalent pour afficher du texte
   - Message affiché au lancement de l'app

2. **Boucle principale**
   - Boucle infinie avec lecture des inputs
   - Quitter proprement l'app sur une combinaison de touches

3. **Packaging**
   - `CMakeLists.txt` configuré avec titre, ID unique, version
   - Génération du `.vpk` via les macros VitaSDK

## 6. Critères de succès
- [ ] Le projet compile sans erreur avec CMake
- [ ] Le `.vpk` s'installe correctement via VitaShell
- [ ] L'app se lance et affiche le message attendu
- [ ] L'app peut être fermée proprement depuis la console

## 7. Plan / Étapes

| Étape | Description | Temps estimé |
|---|---|---|
| 1 | Installation VitaSDK + CMake | 20-40 min |
| 2 | Structure du projet (CMakeLists.txt, sce_sys/) | 15-30 min |
| 3 | Écriture du code de base | 20-40 min |
| 4 | Compilation et débogage | 15-45 min |
| 5 | Transfert + test sur console | 10-15 min |

**Total estimé : 1h20 à 3h**

## 8. Risques identifiés
- Erreurs d'environnement (variable `VITASDK` mal configurée)
- Version de firmware Vita non compatible avec la méthode de transfert choisie
- Premher essai de compilation qui échoue (dépendances manquantes)

## 9. Prochaines étapes après le MVP
- Ajouter `libvita2d` pour du rendu graphique
- Ajouter la gestion d'inputs plus riche (joystick analogique, touch)
- Explorer le réseau (HTTP) ou l'audio selon les besoins futurs