#!/usr/bin/env python3
"""Read-only sync of Garmin Connect data into local markdown + JSON.

Nothing here writes to Garmin Connect and nothing is sent anywhere except
Garmin's own API (required to fetch the data). All output stays under
garmin/ next to this script. The login session token is stored at
~/.garminconnect so other local tools (e.g. an MCP server) can reuse it.
"""

from __future__ import annotations

import argparse
import getpass
import json
import re
import sys
import time
from datetime import date, timedelta
from pathlib import Path
from typing import Any

from garminconnect import Garmin, GarminConnectAuthenticationError

TOKENSTORE = Path.home() / ".garminconnect"
ROOT = Path(__file__).resolve().parent
OUT_DIR = ROOT / "garmin"
DAILY_DIR = OUT_DIR / "daily"
ACTIVITIES_DIR = OUT_DIR / "activities"
DATA_FILE = OUT_DIR / "data.json"
FOOD_DIR = ROOT / "food"

DEFAULT_DAYS = 100


def login(force_interactive: bool) -> Garmin:
    if not force_interactive:
        try:
            client = Garmin()
            client.login(tokenstore=str(TOKENSTORE))
            return client
        except Exception as exc:
            print(f"Konnte gespeicherte Garmin-Session nicht laden: {exc}")
            print("Einmalig anmelden mit: sync_garmin.py --login")
            sys.exit(1)

    email = input("Garmin E-Mail: ").strip()
    password = getpass.getpass("Garmin Passwort: ")

    def prompt_mfa() -> str:
        return input("2FA-Code: ").strip()

    client = Garmin(email=email, password=password, prompt_mfa=prompt_mfa)
    try:
        client.login(tokenstore=str(TOKENSTORE))
    except GarminConnectAuthenticationError as exc:
        print(f"Login fehlgeschlagen: {exc}")
        sys.exit(1)
    print(f"Login ok. Session gespeichert unter {TOKENSTORE}")
    return client


def dig(d: Any, *path: str, default: str = "n/a") -> Any:
    cur = d
    for key in path:
        if not isinstance(cur, dict) or cur.get(key) is None:
            return default
        cur = cur[key]
    return cur


def fmt_seconds(seconds: Any) -> str:
    if not isinstance(seconds, (int, float)):
        return "n/a"
    seconds = int(seconds)
    h, rem = divmod(seconds, 3600)
    m, s = divmod(rem, 60)
    if h:
        return f"{h}h {m:02d}min"
    return f"{m}min"


def fmt_hms(seconds: Any) -> str:
    if not isinstance(seconds, (int, float)):
        return "n/a"
    seconds = int(seconds)
    h, rem = divmod(seconds, 3600)
    m, s = divmod(rem, 60)
    if h:
        return f"{h}:{m:02d}:{s:02d}"
    return f"{m}:{s:02d}"


def slugify(text: str) -> str:
    text = re.sub(r"[^\w\- ]", "", text, flags=re.UNICODE).strip().lower()
    return re.sub(r"[\s_]+", "-", text) or "activity"


def fetch_day(client: Garmin, day: str) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, fn in (
        ("summary", lambda: client.get_user_summary(day)),
        ("sleep", lambda: client.get_sleep_data(day)),
        ("hrv", lambda: client.get_hrv_data(day)),
        ("weigh_in", lambda: client.get_daily_weigh_ins(day)),
        ("fitness_age", lambda: client.get_fitnessage_data(day)),
    ):
        try:
            result[key] = fn()
        except Exception as exc:
            result[key] = None
            print(f"  [{day}] {key}: nicht verfuegbar ({exc})")
    return result


def fetch_max_metrics_range(client: Garmin, start: str, end: str) -> dict[str, Any]:
    """VO2max wird von Garmin nur alle paar Tage neu berechnet, nicht taeglich.
    Eine Bereichsabfrage (statt 100x Einzeltag) liefert alle Tage, an denen
    tatsaechlich ein neuer Wert vorliegt; dazwischen wird der letzte bekannte
    Wert fortgeschrieben (siehe latest_on_or_before).
    """
    url = f"{client.garmin_connect_metrics_url}/{start}/{end}"
    try:
        entries = client.connectapi(url) or []
    except Exception as exc:
        print(f"  VO2max-Verlauf nicht verfuegbar ({exc})")
        return {}
    by_date = {}
    for entry in entries:
        cal_date = dig(entry, "generic", "calendarDate", default=None)
        if cal_date:
            by_date[cal_date] = entry
    return by_date


def latest_on_or_before(entries_by_date: dict[str, Any], day: str) -> dict[str, Any]:
    candidates = [d for d in entries_by_date if d <= day]
    if not candidates:
        return {}
    return entries_by_date[max(candidates)]


def render_daily_markdown(day: str, d: dict[str, Any], max_metrics_entry: dict[str, Any]) -> str:
    summary = d.get("summary") or {}
    sleep = d.get("sleep") or {}
    sleep_dto = sleep.get("dailySleepDTO") or {}
    hrv = d.get("hrv") or {}
    hrv_summary = hrv.get("hrvSummary") or {}
    weigh_in = d.get("weigh_in") or {}
    weight_entries = weigh_in.get("dateWeightList") or []
    weight_entry = weight_entries[0] if weight_entries else {}
    vo2max_running = dig(max_metrics_entry, "generic", "vo2MaxValue")
    vo2max_cycling = dig(max_metrics_entry, "cycling", "vo2MaxValue")
    vo2max_date = dig(max_metrics_entry, "generic", "calendarDate", default=None)
    fitness_age = d.get("fitness_age") or {}
    fitness_age_value = fitness_age.get("fitnessAge")
    fitness_age_str = f"{fitness_age_value:.1f}" if isinstance(fitness_age_value, (int, float)) else "n/a"

    sleep_score = dig(sleep_dto, "sleepScores", "overall", "value")
    weight_kg = f"{weight_entry['weight'] / 1000:.1f} kg" if isinstance(weight_entry.get("weight"), (int, float)) else "n/a"
    lines = [
        f"# {day}",
        "",
        "## Schlaf",
        f"- Score: {sleep_score}",
        f"- Gesamt: {fmt_seconds(sleep_dto.get('sleepTimeSeconds'))}"
        f" (Tief {fmt_seconds(sleep_dto.get('deepSleepSeconds'))},"
        f" Leicht {fmt_seconds(sleep_dto.get('lightSleepSeconds'))},"
        f" REM {fmt_seconds(sleep_dto.get('remSleepSeconds'))},"
        f" Wach {fmt_seconds(sleep_dto.get('awakeSleepSeconds'))})",
        "",
        "## HRV",
        f"- Letzte Nacht: {dig(hrv_summary, 'lastNightAvg')} ms"
        f" (Status: {dig(hrv_summary, 'status')})",
        f"- Wochendurchschnitt: {dig(hrv_summary, 'weeklyAvg')} ms",
        "",
        "## Ruhepuls",
        f"- {dig(summary, 'restingHeartRate')} bpm",
        "",
        "## Body Battery",
        f"- Aufgeladen: +{dig(summary, 'bodyBatteryChargedValue')},"
        f" Verbraucht: -{dig(summary, 'bodyBatteryDrainedValue')}",
        f"- Spanne: {dig(summary, 'bodyBatteryLowestValue')}"
        f" - {dig(summary, 'bodyBatteryHighestValue')}",
        "",
        "## Stress",
        f"- Durchschnitt: {dig(summary, 'averageStressLevel')}",
        f"- Maximum: {dig(summary, 'maxStressLevel')}",
        "",
        "## Steps",
        f"- {dig(summary, 'totalSteps')} / {dig(summary, 'dailyStepGoal')}",
        "",
        "## Kalorien",
        f"- Gesamt: {dig(summary, 'totalKilocalories')} kcal"
        f" (aktiv: {dig(summary, 'activeKilocalories')} kcal)",
        "",
        "## Gewicht & Koerper",
        f"- Gewicht: {weight_kg}",
        f"- BMI: {dig(weight_entry, 'bmi')}",
        f"- Koerperfett: {dig(weight_entry, 'bodyFat')} %",
        "",
        "## VO2max",
        f"- Laufen: {vo2max_running}, Radfahren: {vo2max_cycling}"
        + (f" (Stand: {vo2max_date})" if vo2max_date else ""),
        "",
        "## Fitness-Alter",
        f"- {fitness_age_str} (chronologisch: {dig(fitness_age, 'chronologicalAge')})",
        "",
        "---",
        "_Rohdaten: siehe data.json_",
        "",
    ]
    return "\n".join(lines)


def render_activity_markdown(a: dict[str, Any]) -> str:
    name = a.get("activityName") or "Aktivitaet"
    start = a.get("startTimeLocal", "n/a")
    activity_type = dig(a, "activityType", "typeKey")
    duration = fmt_hms(a.get("duration"))
    distance_km = f"{a['distance'] / 1000:.2f} km" if isinstance(a.get("distance"), (int, float)) else "n/a"
    avg_speed = a.get("averageSpeed")
    pace = "n/a"
    if isinstance(avg_speed, (int, float)) and avg_speed > 0:
        sec_per_km = 1000 / avg_speed
        pace = f"{fmt_hms(sec_per_km)} /km"

    lines = [
        f"# {name} - {start}",
        "",
        f"- Typ: {activity_type}",
        f"- Dauer: {duration}",
        f"- Distanz: {distance_km}",
        f"- Kalorien: {dig(a, 'calories')}",
        f"- Puls: Ø {dig(a, 'averageHR')} bpm, Max {dig(a, 'maxHR')} bpm",
        f"- Hoehenmeter: {dig(a, 'elevationGain')} m",
        f"- Trainingseffekt (aerob/anaerob): {dig(a, 'aerobicTrainingEffect')} / {dig(a, 'anaerobicTrainingEffect')}",
        f"- Pace: {pace}",
        "",
        f"_Activity ID: {dig(a, 'activityId')}_",
        "",
    ]
    return "\n".join(lines)


def ensure_food_note(day: str) -> None:
    """Legt food/<day>.md an, falls sie noch nicht existiert. Ueberschreibt nie
    bestehenden Inhalt - das ist der Ess-Log, den der Nutzer von Hand befuellt.
    """
    path = FOOD_DIR / f"{day}.md"
    if not path.exists():
        path.write_text(f"# {day}\n\n")


def load_data_file() -> dict[str, Any]:
    if DATA_FILE.exists():
        try:
            data = json.loads(DATA_FILE.read_text())
            data.setdefault("max_metrics", {})
            return data
        except json.JSONDecodeError:
            pass
    return {"daily": {}, "activities": {}, "max_metrics": {}}


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--days", type=int, default=DEFAULT_DAYS, help=f"Anzahl Tage rueckwirkend (Default: {DEFAULT_DAYS})")
    parser.add_argument("--login", action="store_true", help="Interaktiven Garmin-Login erzwingen")
    args = parser.parse_args()

    DAILY_DIR.mkdir(parents=True, exist_ok=True)
    ACTIVITIES_DIR.mkdir(parents=True, exist_ok=True)
    FOOD_DIR.mkdir(parents=True, exist_ok=True)

    client = login(args.login)

    today = date.today()
    start_day = today - timedelta(days=args.days - 1)
    days = [(start_day + timedelta(days=i)).isoformat() for i in range(args.days)]

    ensure_food_note(today.isoformat())

    data = load_data_file()

    print("Hole VO2max-Verlauf...")
    data["max_metrics"].update(fetch_max_metrics_range(client, days[0], days[-1]))

    print(f"Hole Wellness-Daten fuer {len(days)} Tage ({days[0]} .. {days[-1]})...")
    for day in days:
        raw = fetch_day(client, day)
        data["daily"][day] = raw
        max_metrics_entry = latest_on_or_before(data["max_metrics"], day)
        (DAILY_DIR / f"{day}.md").write_text(render_daily_markdown(day, raw, max_metrics_entry))
        time.sleep(0.2)

    print("Hole Aktivitaeten...")
    try:
        activities = client.get_activities_by_date(days[0], days[-1])
    except Exception as exc:
        print(f"  Aktivitaeten nicht verfuegbar ({exc})")
        activities = []

    for a in activities:
        activity_id = a.get("activityId")
        data["activities"][str(activity_id)] = a
        start_local = a.get("startTimeLocal", "")
        day_part = start_local.split(" ")[0] if start_local else "unknown"
        slug = slugify(a.get("activityName") or "activity")
        filename = f"{day_part}_{activity_id}_{slug}.md"
        (ACTIVITIES_DIR / filename).write_text(render_activity_markdown(a))

    DATA_FILE.write_text(json.dumps(data, indent=2, ensure_ascii=False, sort_keys=True))

    print(f"Fertig: {len(days)} Tagesnotizen, {len(activities)} Aktivitaeten -> {OUT_DIR}")


if __name__ == "__main__":
    main()
