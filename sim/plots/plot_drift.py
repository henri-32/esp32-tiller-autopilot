from __future__ import annotations

import importlib
import subprocess
import sys
from pathlib import Path
from typing import Tuple


def resolve_repo_root() -> Path:
    """Resolve repository root by locating the sim analysis requirements file."""
    current = Path(__file__).resolve()
    for parent in current.parents:
        if (parent / "sim" / "analysis" / "requirements.txt").exists():
            return parent
    return current.parents[2]


def resolve_paths() -> Tuple[Path, Path, Path]:
    repo_root = resolve_repo_root()
    csv_path = repo_root / "sim" / "sim_output" / "drift_sim.csv"
    requirements_path = repo_root / "sim" / "analysis" / "requirements.txt"
    return repo_root, csv_path, requirements_path


def _install_requirements(requirements_path: Path) -> None:
    cmd = [sys.executable, "-m", "pip",
           "install", "-r", str(requirements_path)]
    print("Missing dependency detected. Trying automatic install via:")
    print(f"  {' '.join(cmd)}")
    subprocess.run(cmd, check=True)


def load_dependencies(requirements_path: Path):
    try:
        pandas = importlib.import_module("pandas")
        pyplot = importlib.import_module("matplotlib.pyplot")
        return pandas, pyplot
    except ModuleNotFoundError as exc:
        missing_name = exc.name or "unknown"
        if not requirements_path.exists():
            raise SystemExit(
                "Missing Python dependency and requirements file not found.\n"
                f"Missing module: {missing_name}\n"
                f"Interpreter: {sys.executable}\n"
                "Expected requirements file: "
                f"{requirements_path}"
            ) from exc

        try:
            _install_requirements(requirements_path)
        except (subprocess.CalledProcessError, OSError) as install_exc:
            raise SystemExit(
                "Automatic dependency installation failed.\n"
                f"Missing module: {missing_name}\n"
                f"Interpreter: {sys.executable}\n"
                "Try manually with:\n"
                f"  python -m pip install -r sim/analysis/requirements.txt\n"
                f"or:\n"
                f"  {sys.executable} -m pip install -r {requirements_path}"
            ) from install_exc

        try:
            pandas = importlib.import_module("pandas")
            pyplot = importlib.import_module("matplotlib.pyplot")
            return pandas, pyplot
        except ModuleNotFoundError as retry_exc:
            retry_missing = retry_exc.name or missing_name
            raise SystemExit(
                "Dependencies are still missing after install attempt.\n"
                f"Missing module: {retry_missing}\n"
                f"Interpreter: {sys.executable}\n"
                "Try manually with:\n"
                "  python -m pip install -r sim/analysis/requirements.txt"
            ) from retry_exc


def unwrap_degrees(series):
    values = [float(v) for v in series.tolist()]
    if not values:
        return []

    unwrapped = [values[0]]
    offset = 0.0

    for i in range(1, len(values)):
        delta = values[i] - values[i - 1]
        if delta > 180.0:
            offset -= 360.0
        elif delta < -180.0:
            offset += 360.0
        unwrapped.append(values[i] + offset)

    return unwrapped


def plot_drift(df, plt) -> None:
    time_s = df["time_ms"] / 1000.0
    heading_plot = unwrap_degrees(df["heading_deg"])
    error_plot = df["error_deg"]
    median_plot = unwrap_degrees(df["median_deg"])
    omega_raw = df["angular_velocity_deg_s"]
    omega_smooth = omega_raw.rolling(window=5, center=True, min_periods=1).mean()

    plt.figure(figsize=(12, 6))
    plt.plot(
        time_s,
        heading_plot,
        label="Heading (sampled, steps)",
        drawstyle="steps-post",
    )
    plt.plot(time_s, error_plot, label="Error (raw)")
    plt.plot(time_s, median_plot, label="Median (ObservationBuffer, unwrapped)")
    plt.plot(
        time_s,
        omega_smooth,
        label="Angular Velocity (smoothed, deg/s)",
        linestyle="-.",
    )
    plt.plot(
        time_s,
        omega_raw,
        label="Angular Velocity (raw)",
        linestyle="--",
        alpha=0.25,
        linewidth=1.0,
    )
    if "trim_state" in df.columns:
      plt.plot(
          time_s,
          df["trim_state"],
          label="Trim State",
          linestyle=":",
      )

    intent = df[df["intent"] == 1]
    intent_right = intent[intent["dir"] == "Right"]
    intent_left = intent[intent["dir"] == "Left"]

    plt.scatter(
        intent_right["time_ms"] / 1000.0,
        [heading_plot[idx] for idx in intent_right.index],
        color="green",
        marker="x",
        label="Intent Right",
    )
    plt.scatter(
        intent_left["time_ms"] / 1000.0,
        [heading_plot[idx] for idx in intent_left.index],
        color="red",
        marker="x",
        label="Intent Left",
    )

    plt.xlabel("Time (s)")
    plt.ylabel("Angle (deg)")
    plt.legend()
    plt.grid(True)
    plt.show()


def main() -> int:
    _, csv_path, requirements_path = resolve_paths()
    pd, plt = load_dependencies(requirements_path)

    if not csv_path.exists():
        raise SystemExit(f"Simulation output not found: {csv_path}")

    try:
        df = pd.read_csv(csv_path)
    except Exception as exc:
        raise SystemExit(
            f"Failed to read simulation output '{csv_path}': {exc}") from exc

    if df.empty:
        raise SystemExit(f"Simulation output is empty: {csv_path}")

    required_columns = {
        "time_ms",
        "heading_deg",
        "angular_velocity_deg_s",
        "error_deg",
        "median_deg",
        "intent",
    }
    missing_columns = sorted(required_columns - set(df.columns))
    if missing_columns:
        raise SystemExit(
            "Simulation output is missing required columns: "
            f"{', '.join(missing_columns)}"
        )

    plot_drift(df, plt)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
