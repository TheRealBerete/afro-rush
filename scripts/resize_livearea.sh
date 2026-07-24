#!/bin/bash
set -e
cd /mnt/d/Bérété/projets/psvita

convert assets/icon0.png -resize 128x128 sce_sys/icon0.png

convert assets/bg.png -resize 840x500^ -gravity center -extent 840x500 sce_sys/livearea/contents/bg.png

convert assets/startup.png -resize 280x158! sce_sys/livearea/contents/startup.png

echo "--- resultats ---"
identify sce_sys/icon0.png sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/startup.png
