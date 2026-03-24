# Panacée

A dual-component application for optimizing healthcare facility placement across metropolitan France using genetic algorithms.

## Project Structure

```
projet-panacée/
├── apps/
│   ├── panacee-genetics/       # C genetic algorithm engine
│   │   └── src/
│   │       ├── assets/         # French municipalities CSV (INSEE, coords, population)
│   │       ├── datas/          # Data structures (hospital, town, fitness)
│   │       └── main.c
│   └── panacee-documents/      # Python analysis/documentation layer
│       └── main.py
├── docs/
│   ├── instructions/           # Project instructions and recommendations (PDF)
│   └── project/                # Build documentation
└── Makefile                    # Top-level build orchestrator
```

## Components

**panacee-genetics** — C application implementing a genetic algorithm to evaluate optimal hospital distribution. Works with French municipality data (INSEE codes, geographic coordinates, population counts) and scores candidates using fitness metrics: hospital coverage, UHC (urgent care) access, and travel distances.

**panacee-documents** — Python application for analysis and documentation of results produced by the genetics engine.

## Usage

```sh
make init     # Install Python dependencies
make build    # Compile the C application
make run      # Run both components
make clean    # Remove object files
make fclean   # Remove object files and binary
make re       # Full rebuild
```

## References

- [Panacée (Wikipedia)](https://fr.wikipedia.org/wiki/Panac%C3%A9e)
