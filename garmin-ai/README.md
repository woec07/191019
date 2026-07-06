# garmin-ai

Lokales, read-only Setup zum Abgreifen eigener Garmin-Connect-Daten. Es wird
nichts an externe Server geschickt außer an Garmin selbst (um deine eigenen
Daten zu holen) — alles landet nur lokal auf dieser Maschine.

## Was hier drin ist

- `sync_garmin.py` — das Sync-Skript
- `.venv/` — Python-venv mit `python-garminconnect`
- `garmin/` — die gesyncten Daten
  - `daily/YYYY-MM-DD.md` — eine Recovery-Notiz pro Tag (Schlaf, HRV,
    Ruhepuls, Body Battery, Stress, Steps, Kalorien, Gewicht, VO2max,
    Fitness-Alter). Training Readiness wird nicht gesynct — die Venu 3s
    liefert dafür keine Daten.
  - `activities/*.md` — eine Datei pro Workout
  - `data.json` — alle Rohdaten als JSON
- `food/YYYY-MM-DD.md` — dein Ess-Log. Wird beim Sync automatisch für den
  aktuellen Tag angelegt (nur mit Datums-Header, sonst leer); bestehende
  Dateien werden nie überschrieben. Einfach roh reinschreiben, Zeile für
  Zeile, was du isst/trinkst — kein Format, keine Kalorien nötig.
- `~/.garminconnect` (außerhalb dieses Ordners) — der Login-Token
- `CLAUDE.md` — Kontext für Claude Code (Profil, Ziele, Trainingsplan) plus
  Anweisung, vor Trainingsfragen `garmin/daily/` und `garmin/activities/`,
  und vor Ernährungsfragen zusätzlich `food/` zu lesen

## 1. Automatischer Sync (läuft schon)

Ein systemd-User-Timer synct täglich um **6:00 Uhr lokal** von selbst
(± bis zu 5 Min. Zufallsverzögerung). Nichts zu tun.

```bash
systemctl --user status garmin-sync.timer       # nächster geplanter Lauf
systemctl --user list-timers garmin-sync.timer  # Übersicht
journalctl --user -u garmin-sync.service -n 30  # Log des letzten Laufs
```

## 2. Manuell syncen

Falls du zwischendurch aktuelle Daten willst (z. B. direkt nach einem Lauf):

```bash
cd /home/matthias/git/garmin-ai
.venv/bin/python sync_garmin.py            # Standard: 100 Tage zurück
.venv/bin/python sync_garmin.py --days 7   # nur letzte 7 Tage (schneller)
```

Oder über systemd:

```bash
systemctl --user start garmin-sync.service
```

## 3. Daten ansehen

Einfach die Markdown-Dateien öffnen, z. B.:

```bash
cat garmin/daily/2026-07-07.md
ls garmin/activities/
```

`garmin/data.json` enthält alles als Rohdaten, falls du selbst mit
Python/`jq` auswerten willst.

## 4. Mit Claude Code für Trainingsfragen nutzen

Der eigentliche Zweck: Claude-Code-Session in diesem Ordner starten und
fragen, z. B.:

> "Sollte ich heute Intervalle laufen oder eher locker traben?"

Dank `CLAUDE.md` liest Claude automatisch `garmin/daily/` und
`garmin/activities/`, bewertet HRV-Trend, Body Battery und Schlaf und
gibt eine Empfehlung im Kontext deiner Ziele und deines Trainingsplans.

## 5. Login erneuern (falls nötig)

Der Token in `~/.garminconnect` wird normalerweise automatisch erneuert.
Falls er doch mal abläuft oder ungültig wird:

```bash
cd /home/matthias/git/garmin-ai
.venv/bin/python sync_garmin.py --login
```

Fragt nacheinander nach E-Mail, Passwort (unsichtbare Eingabe) und ggf.
2FA-Code.

## Timer verwalten

```bash
systemctl --user stop garmin-sync.timer       # pausieren
systemctl --user disable --now garmin-sync.timer  # komplett abschalten
systemctl --user enable --now garmin-sync.timer   # wieder aktivieren
```

Unit-Dateien liegen unter `~/.config/systemd/user/garmin-sync.{service,timer}`.
