from pathlib import Path

try:
    import pandas as pd
    import matplotlib.pyplot as plt
    
except ModuleNotFoundError as exc:
    raise SystemExit(
        "Missing Python dependency. Use "
        "'./sim/analysis/venv/Scripts/python.exe sim/plots/plot_drift.py' "
        "or install requirements in your active interpreter."
    ) from exc

SIM_ROOT = Path(__file__).resolve().parents[1]
CSV_PATH = SIM_ROOT / "sim_output" / "drift_sim.csv"

if not CSV_PATH.exists():
    raise SystemExit(f"Simulation output not found: {CSV_PATH}")

df = pd.read_csv(CSV_PATH)

plt.figure(figsize=(12,6))

plt.plot(df["time_ms"]/1000, df["heading_deg"], label="Heading")
plt.plot(df["time_ms"]/1000, df["error_deg"], label="Error")

intent = df[df["intent"] == 1]
plt.scatter(intent["time_ms"]/1000,
            intent["heading_deg"],
            color="red",
            marker="x",
            label="Intent")

plt.xlabel("Time (s)")
plt.legend()
plt.grid(True)
plt.show()
