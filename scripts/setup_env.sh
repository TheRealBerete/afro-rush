#!/bin/bash
set -e

chown -R berete:berete /usr/local/vitasdk

BASHRC=/home/berete/.bashrc

if ! grep -q "VITASDK=/usr/local/vitasdk" "$BASHRC" 2>/dev/null; then
  {
    echo ''
    echo '# VitaSDK'
    echo 'export VITASDK=/usr/local/vitasdk'
    echo 'export PATH=$VITASDK/bin:$PATH'
  } >> "$BASHRC"
  echo "Ajoute a $BASHRC"
else
  echo "Deja present dans $BASHRC"
fi

chown berete:berete "$BASHRC"
