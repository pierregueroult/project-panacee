# Panacée

A dual-component application for optimizing healthcare facility placement across metropolitan France using genetic algorithms.

## Project Structure

```/dev/null/tree.txt#L1-18
projet-panacée/
├── apps/
│   ├── panacee-genetics/       # C genetic algorithm engine
│   │   ├── src/
│   │   │   ├── assets/         # Assets files containing data
│   │   │   ├── datas/          # Data structures (hospital, town, fitness)
│   │   │   └── main.c
│   │   ├── bin/                # Compiled object files (.o)
│   │   ├── makefile            # C build and run targets
│   │   └── panacee             # Output binary
│   └── panacee-documents/      # Python analysis/documentation layer
│       ├── main.py
│       └── requirement.txt
├── docs/
│   ├── instructions/           # Project instructions and recommendations (PDF)
│   └── project/                # Build documentation
└── makefile                    # Top-level build orchestrator
```

## Components

**panacee-genetics** — C application implementing a genetic algorithm to evaluate optimal hospital distribution. It works with French municipality data (INSEE codes, geographic coordinates, population counts) and scores candidates using fitness metrics: hospital coverage, UHC (urgent care) access, and travel distances.

**panacee-documents** — Python application for analysis and documentation of results produced by the genetics engine.

## Usage (from project root)

```sh
make help     # Show available top-level targets
make init     # Install Python dependencies
make build    # Compile the C application (apps/panacee-genetics)
make run      # Build, run C binary, then run Python app
make clean    # Clean C build artifacts
make fclean   # Full clean (artifacts + binary)
make re       # Full rebuild (fclean then build)
```

## References

- [Panacée (Wikipedia)](https://fr.wikipedia.org/wiki/Panac%C3%A9e)
