# Panacée

Repository: https://github.com/pierregueroult/project-panacee

## Project Structure

```
project-panacee/
├── apps/
│   ├── panacee-genetics/           # C genetic algorithm engine (MVC)
│   │   ├── src/
│   │   │   ├── model/              # Domain logic and data
│   │   │   │   ├── town/           # Town entity (INSEE code, coordinates, population)
│   │   │   │   ├── hospital/       # Hospital entity (INSEE code, bed count)
│   │   │   │   ├── individual/     # A candidate solution (set of hospital towns)
│   │   │   │   ├── population/     # Population of individuals
│   │   │   │   ├── fitness/        # Fitness value object (scoring metrics)
│   │   │   │   ├── genetic/        # Evolution operators (selection, crossover, mutation)
│   │   │   │   ├── io/             # CSV parser and result exporters
│   │   │   │   ├── config.h        # Algorithm tuning parameters and domain constants
│   │   │   │   └── context.h       # Problem environment bundle
│   │   │   ├── view/               # Rendering (MLV map, console output, colors)
│   │   │   ├── controller/         # Orchestration of the genetic run
│   │   │   ├── util/               # Checked allocation helpers
│   │   │   └── main.c
│   │   ├── Doxyfile                # Doxygen documentation config
│   │   ├── makefile                # C build and run targets
│   │   └── panacee                 # Output binary
│   ├── panacee-pdf-generator/      # Python app: per-department PDF report of results
│   └── panacee-map/                # Interactive web map (Java HTTP server + JS/Leaflet front)
├── data/
│   └── output/                     # CSV results and generated PDF
├── docs/
│   ├── instructions/               # Project instructions and recommendations (PDF)
│   └── project/                    # Build and algorithm documentation
└── makefile                        # Top-level build orchestrator
```

### Architecture (panacee-genetics)

Le code source suit une organisation **MVC** :

- **`model/`**: entités métier, opérateurs génétiques, lecture/écriture des données ; aucune dépendance vers l'affichage
- **`view/`**: rendu graphique via la bibliothèque MLV (carte de France) et sortie console
- **`controller/`**: pilote l'exécution : initialisation, boucle d'évolution, recherche locale finale, export

## Components

**panacee-genetics**: C application implementing a genetic algorithm to evaluate optimal hospital distribution. It works with French municipality data (INSEE codes, geographic coordinates, population counts) and scores candidates using the fitness function given in the project instructions: population coverage, hospital cost penalty, and CHRU bonus.

**panacee-pdf-generator**: Python application that turns the CSV results into a per-department PDF report.

**panacee-map**: interactive web map (Java HTTP server, Leaflet front end) to explore the resulting hospital placement.

## Usage (from project root)

```sh
make help     # Show available top-level targets
make init     # Install Python dependencies
make build    # Compile the C application (apps/panacee-genetics)
make run      # Build, run C binary, generate the PDF, then serve the map
make clean    # Clean C build artifacts
make fclean   # Full clean (artifacts + binary)
make re       # Full rebuild (fclean then build)
```

## Documentation

- [`docs/project/algorithme-genetique-explique.md`](docs/project/algorithme-genetique-explique.md) — l'algorithme génétique expliqué pas à pas (français)
- [`docs/project/genetics.md`](docs/project/genetics.md) — technical description of the genetic engine
- `make docs` in `apps/panacee-genetics/` — generate the Doxygen API documentation

## References

- [Panacée (Wikipedia)](https://fr.wikipedia.org/wiki/Panac%C3%A9e)
