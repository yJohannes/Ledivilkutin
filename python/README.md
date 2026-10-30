# Python / Running simulations

## Setup

1. Navigate to the `python/` directory:
   ```bash
   cd python
   ```
2. Install dependencies:
   ```bash
   poetry install
   ```

If Poetry uses the wrong Python interpreter, select one explicitly:

```bash
poetry env use <path-to-python.exe>
poetry install
```

## Run simulations

From the `python/` directory, run either simulation:

Particle based simulation:
```bash
poetry run python src/particle/main.py
```

Grid based simulation (abandoned):

```bash
poetry run python src/grid/main.py
```
