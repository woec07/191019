# garmin-ai

Lokales, read-only Setup zum Abgreifen eigener Garmin-Connect-Daten (Aktivitäten
+ Recovery: Schlaf, HRV, Ruhepuls, Body Battery, Stress, Steps). Sync-Skript:
`sync_garmin.py`. Daten landen unter `garmin/` (`daily/` = eine Markdown-Notiz
pro Tag, `activities/` = eine pro Workout, `data.json` = Rohdaten). Login-Token
liegt in `~/.garminconnect`.

> **Hinweis Kalorien:** Die Venu 3s trackt aktive + Ruhe-Kalorien, und
> `python-garminconnect` kann sie abrufen (pro Aktivität sowie Tagessumme:
> `active`, `bmr`/Ruhe, `total`). Das aktuelle `sync_garmin.py` schreibt sie noch
> **nicht** in die Tagesnotiz — bei Bedarf `active_calories` und `total_calories`
> zu den täglichen Wellness-Feldern hinzufügen. Behandle diese Werte als grobe
> Trend-Schätzung, nicht als exakte Zahl für Defizit-Rechnungen.

## Person
- Alter: 24 (männlich)

## Uhr
- Modell: Garmin Venu 3s
- **Verfügbar:** HRV Status (7-Tage-Baseline), Body Battery, Schlaf (+ Score),
  Stress, Ruhepuls, Steps, Kalorien (Schätzung).
- **NICHT verfügbar:** Training Readiness, Training Status, Training Load,
  Running Power. Diese Features hat die Venu 3-Serie nie bekommen (nicht in den
  Specs) — Garmin hat sie Forerunner/Fenix/Venu 4/X1 vorbehalten. Entsprechende
  Felder sind immer leer; **nicht** darauf verlassen. Recovery stattdessen über
  HRV Status + Body Battery + Ruhepuls-Trend + Schlaf lesen.

## Sportarten & Wochenvolumen
- **Mindestens 2× Laufen pro Woche:**
    - 1× Longrun in Z2 (10 km+) zur Verbesserung der Grundlagenausdauer
    - 1× schneller Lauf (Intervalle, Schwellenlauf oder progressiver
      Steigerungslauf)
    - Idealfall: 3× Laufen, d.h. ein zweiter schneller Lauf, sodass Intervalle
      jede Woche fest stehen
- **1× Ganzkörper-Krafttraining** (Pflicht), im Idealfall zusätzlich 1×
  Oberkörper & Core
- **1× Ballsport**, meist statt Laufen — Tennis oder gelegentlich Fußball

## Ziele
- **Herbst 2026:** 5 km in Sub 20:00 (Zieltermin Ende September — Linzer City
  Night Run?)
- **Frühling 2027:** Halbmarathon in Sub 1:40 (besser Sub 1:35)
- **Körperkomposition:** aktuell 88 kg → stabile ~85 kg (moderater Fettabbau),
  Ziel sichtbareres Sixpack / gute Muskulatur überall (Hybridathlet).

## Priorisierung / Periodisierung
Beide Ziele gleichzeitig — und bei nur ~3 kg Delta gut vereinbar: moderater
Fettabbau **unterstützt** die Lauf-Performance (Power-to-weight), er behindert sie
nicht. Kausalrichtung beachten: Das Defizit kommt primär über die Ernährung,
Laufen unterstützt es — **nicht** "mehr laufen, um mehr essen zu dürfen".

Arbeitsregel für den Coach:
- **Jetzt bis Spätsommer:** kleines, konsistentes Defizit Richtung 85 kg, Protein
  hoch, Carbs um die harten Läufe. Trainingsqualität + Recovery haben Vorrang vor
  Abnehm-Tempo.
- **Ab 5k-Aufbau (Spätsommer):** auf Erhalt / leichten Überschuss gehen, damit die
  Tempo-Härte und die Muskulatur voll versorgt sind.
- "Muskel überall aufbauen" läuft im Defizit langsamer — bei diesem kleinen Delta
  okay; echter Aufbau kommt in der Erhalt-/Überschuss-Phase.
- **Konflikt-Regel:** Wenn HRV Status, Schlaf oder Body Battery über mehrere Tage
  abfallen → Defizit ist zu aggressiv. Dann mehr essen (v.a. Carbs um Sessions),
  nicht härter trainieren.

## Ernährung
Ziel: weg von stark raffiniert-/weißbrotlastigen Kohlenhydraten, hin zu höherer
Kohlenhydrat-**Qualität** — bei moderatem Kaloriendefizit Richtung 85 kg.

**Coaching-Leitplanke (wichtig):** Nicht low-carb. Als Ausdauer-/Hybridathlet mit
Sub-20-5k- und Sub-1:40-HM-Zielen sind Kohlenhydrate der primäre Treibstoff für
harte Einheiten. Keine aggressiven Defizite oder Carb-Streichungen empfehlen, die
Trainingsqualität oder Recovery sabotieren.

Praktische Richtung:
- **Getränke:** nur Wasser + schwarzer Kaffee → null Flüssigkalorien. Großer
  Vorteil, so beibehalten.
- **Süßes muss nicht gestrichen werden:** flexibler ~80/20-Ansatz. Basis = gute
  Carbs; Süßes in Maßen, am besten **nach harten Einheiten** (Glykogen-Auffüllung,
  dann am wenigsten problematisch). Verbot triggert eher Heißhunger als Fettabbau.
- **Qualität statt Weißmehl:** Weißbrot → Vollkorn/Sauerteig, Haferflocken,
  Kartoffeln, Reis, Hülsenfrüchte, Obst. Gleicher Treibstoff, mehr Ballaststoffe +
  Mikronährstoffe + stabilerer Blutzucker + länger satt.
- **Timing:** die meisten Carbs um die harten Einheiten legen (vor/nach Tempo,
  Intervallen, Longrun). An reinen Ruhe-/Oberkörpertagen darf der Carb-Anteil
  etwas niedriger liegen.
- **Protein für Kraft + Muskelerhalt im Defizit:** über den Tag verteilt, grober
  Richtwert ~1,6–2,0 g pro kg Körpergewicht. Im Defizit eher am oberen Ende, um
  Muskulatur zu halten.
- **Kein Crash-Diät:** 3 kg brauchen kein hartes Defizit. Klein und konsistent,
  um das Training herum gefüttert.

### Ess-Log (`food/`)
`sync_garmin.py` legt bei jedem Lauf automatisch `food/<YYYY-MM-DD>.md` für den
aktuellen Tag an (nur ein Datums-Header, kein Template) — bestehende Dateien
werden nie überschrieben. Dort trägt der Nutzer roh, Zeile für Zeile, ein was
er isst/trinkt (kein festes Format, keine Kalorien nötig).

**Arbeitsweise bei Ernährungsfragen:** Für den relevanten Zeitraum immer sowohl
`garmin/` (Kalorien-Schätzung, Gewicht, Recovery) **als auch** `food/` (was
tatsächlich gegessen wurde) lesen und gegeneinander abgleichen — z.B. fallende
HRV/Body Battery + laut `food/` wenig/kaum Carbs um harte Einheiten →
Unterfütterung ansprechen.

**Wichtige Grenze, ehrlich kommunizieren:** Garmin trackt nur Kalorien "raus"
(geschätzt), nie Kalorien "rein" — das kommt ausschließlich aus `food/`. Wenn
`food/` für den fraglichen Tag fehlt oder leer ist, das explizit sagen und nur
prinzipien-/sessionbasiert beraten (z.B. "nach harten Einheiten mehr Carbs"),
**nicht** so tun als wäre bekannt was gegessen wurde.

Keine medizinischen Diagnosen — bei echten Ernährungsumstellungen ggf. auf
Fachberatung (Ernährungsberater/Ärztin) verweisen.

## Einschränkungen / Constraints

**Dauerhaft:**
- Schlafrhythmus spät: ca. 2:00 – 9:30 (~7,5 h).
- Bevorzugt Sport am Abend.
- Coach-Hinweis: harte Abend-Einheiten kurz vor spätem Bett können Schlafqualität
  und die nächtliche HRV drücken — bei auffälligen Recovery-Werten mitbedenken.

**Aktuell / temporär** *(nach Abheilung entfernen):*
- ⏳ **Review ~9. Juli 2026:** großer Zeh verletzt (Fußball 5. Juli, starke
  Schwellung), Sportpause bis 8./9. Juli.
  - Wenn Schwellung/Schmerz um den 9. noch deutlich ist oder Abrollen/Auftreten
    nicht normal geht → ärztlich checken (Turf Toe / Haarriss tarnt sich als
    Prellung), bevor wieder hart gelaufen wird.
  - Wiedereinstieg: erst Oberkörper-Gym, Laufen vorsichtig hochfahren (der große
    Zeh trägt beim Abdruck), nicht direkt mit Intervallen.

## Arbeitsweise bei Trainingsfragen
Bevor du eine Frage zu Training, Belastungssteuerung oder Recovery beantwortest,
immer zuerst die aktuellen Daten lesen:
1. `garmin/daily/` (die letzten relevanten Tage) für Schlaf, HRV Status,
   Ruhepuls, Body Battery, Stress, Steps (und Kalorien, falls vorhanden).
2. `garmin/activities/` für die zuletzt absolvierten Workouts (Typ, Dauer,
   Distanz, Puls, Trainingseffekt).

Bewerte jede Trainingsempfehlung immer im Recovery-Kontext, nicht isoliert
(Venu 3s hat **keine** Training Readiness — nutze die Rohsignale):
- **HRV Status:** aktueller Wert vs. 7-Tage-Baseline — balanced / unbalanced /
  low? Trend steigend/fallend/stabil?
- **Body Battery:** Ausgangslevel morgens und Verlauf.
- **Ruhepuls:** erhöht vs. persönlicher Normalwert = mögliches Ermüdungs-/
  Krankheitszeichen.
- **Schlaf:** Dauer und Score der letzten Nächte, nicht nur der letzten Nacht.

Beziehe außerdem immer den Trainingsplan unten ein (v.a. die "Don't do this"-
Reihenfolge-Regeln), wenn du die nächste Einheit vorschlägst.

Wenn die Daten in `garmin/` fehlen oder veraltet sind (z.B. `sync_garmin.py`
wurde länger nicht ausgeführt), das explizit sagen statt mit Annahmen zu
antworten.

---

# 🏋️ Trainingsplan: Hybrid (Kraft & Ausdauer)

## Struktur & Pausen-Regeln (flexibel)
Keine festen Wochentage. Woche flexibel nach diesen goldenen Regeln aufbauen.

**"Don't do this" — was NICHT direkt hintereinander gemacht werden darf:**
1. **Ganzkörper-Gym ➡️ Longrun:** Nach Ganzkörper (Beinfokus) braucht der
   Unterkörper Pause. Kein Longrun am Folgetag.
2. **Ganzkörper-Gym ➡️ Tempo-Lauf:** Intervalle/Schwelle brauchen 100% frische
   Beine. Nie direkt am Tag nach Squats/Kreuzheben hart laufen.
3. **Tempo-Lauf ➡️ Longrun (und umgekehrt):** Zwei harte Laufeinheiten an
   Folgetagen sind tabu. Immer ≥1 Tag Laufpause dazwischen (Oberkörper-Gym oder
   Restday).
4. **Ballsport:** Zählt für die Beine wie eine Tempo-Einheit. Danach keine harten
   Lauf-/Beintrainings am Folgetag.

## 🏃 Laufeinheiten

### 1. Longrun (Pflicht)
Der aerobe Basis-Pusher.
- **Puls-Limit:** strikt < 160 bpm (reines Z2, aerob).
- **Distanz:** mindestens 10 km (Time on Feet).
- **Progression:** wenn 10 km sich gut anfühlen, wöchentlich um 1–1,5 km steigern
  (11,5 → 13 km …).
- **Tempo:** egal — Pace so wählen, dass der Puls unter 160 bleibt (v.a. bei Hitze
  oder Steigungen).

### 2. Tempo-Lauf: Intervalle & Schwelle (alternierend)
Woche A = Intervalle, Woche B = Schwellenlauf.

**Woche A — Intervalle**
- Warm-up: 1 km locker + 3 kurze Steigerungen (50 m).
- Hauptteil: **6× 800 m @ 4:30 min/km**.
- Pause: je 90 s lockeres Traben/Gehen.
- Cool-down: 1 km locker.
- Progression: wenn 4:30 sitzt, Pace schrittweise anziehen (4:25 → 4:20 …).

**Woche B — Schwellenlauf**
Verbessert Laktattoleranz.
- Warm-up: 1 km locker.
- Hauptteil: **20–30 min am Stück @ 165–175 bpm** ("komfortabel hart" — deutlich
  über Z2, ins Schwitzen, aber nicht hechelnd wie bei Intervallen).
- Cool-down: 1 km locker.
- Progression: bei 20 min starten, auf 25 → 30 min am Stück steigern.

## 🏋️ Gym-Einheiten
Schwere Lifts zuerst, solange das Nervensystem frisch ist.

### 1. Ganzkörper (Pflicht)
- Warm-up: 5 min Rudern (Ergo) + Gelenke mobilisieren.
- Kniebeugen: 3× 8–10 (1–2 Wdh. Reserve, kein Muskelversagen).
- Bankdrücken: 3× 8–10.
- Rumänisches Kreuzheben (RDLs): 3× 8–10 (leicht gestreckte Beine, Rückseite/Po).
- Vorgebeugtes Rudern (LH/KH): 3× 8–12 (oberer Rücken).
- Bizeps Curls: 3× 8–12.
- Planks: 3× max. halten.

### 2. Oberkörper & Core (Ideal-Woche)
- Warm-up: 5 min Rudern + Schultern kreisen.
- Bankdrücken: 3× 8–10.
- Latzug / Klimmzüge: 3× 8–12.
- Military Press (stehend): 3× 8–10.
- Bizeps Curls: 3× 8–12.
- Bauch: Reverse Crunches **oder** Dead Bugs: 3× 12–15.
- Bauch: Crunches am Kabel **oder** Bauchpresse: 3× 10–15.
- Planks: 3× max. halten.

## 💡 Key Rules im Gym
1. **Pausen:** bei Kniebeugen, Kreuzheben, Bankdrücken, Military Press strikt
   2,5–3 min. Bei Armen/Bauch reichen 90 s.
2. **Progression:** sobald in allen 3 Sätzen die obere Wdh-Grenze sauber steht
   (z.B. 12 Curls), beim nächsten Mal minimal Gewicht drauf.
