# Panacée

Optimisation de la répartition des hôpitaux sur le territoire français à l'aide d'un algorithme génétique.

Le projet évalue des placements candidats d'hôpitaux à partir des données des communes françaises (codes INSEE, coordonnées géographiques, population) et les note selon la fonction de fitness du sujet : couverture de la population, pénalité de coût des hôpitaux et bonus CHRU.

## Composants

| Application | Description |
|---|---|
| [`apps/panacee-genetics`](apps/panacee-genetics) | Moteur génétique en C (architecture MVC, rendu carte via MLV) |
| [`apps/panacee-pdf-generator`](apps/panacee-pdf-generator) | Application Python qui génère un rapport PDF par département à partir des CSV |
| [`apps/panacee-map`](apps/panacee-map) | Carte web interactive (serveur HTTP Java + front Leaflet) |

Les résultats (CSV et PDF générés) sont écrits dans [`data/output/`](data/output).

## Architecture (panacee-genetics)

Le code source suit une organisation **MVC** :

- **`model/`** : entités métier (commune, hôpital, individu, population), opérateurs génétiques, lecture/écriture des données — aucune dépendance vers l'affichage
- **`view/`** : rendu graphique via la bibliothèque MLV (carte de France) et sortie console
- **`controller/`** : pilote l'exécution — initialisation, boucle d'évolution, recherche locale finale, export

## Utilisation (depuis la racine)

```sh
make help     # Affiche les cibles disponibles
make init     # Installe les dépendances Python (virtualenv)
make build    # Compile l'application C (nécessite libMLV)
make run      # Compile, exécute le binaire C, génère le PDF, puis sert la carte
make clean    # Supprime les fichiers de build
make fclean   # Nettoyage complet (artefacts + binaire)
make re       # Rebuild complet (fclean puis build)
```

## Documentation

- [`docs/project/genetics.md`](docs/project/genetics.md) — description technique du moteur génétique
- [`docs/project/makefile.md`](docs/project/makefile.md) — fonctionnement des Makefiles
- [`docs/instructions/`](docs/instructions) — sujet et recommandations du projet (PDF)
- `make docs` dans `apps/panacee-genetics/` — génère la documentation API Doxygen

## Référence

- [Panacée (Wikipédia)](https://fr.wikipedia.org/wiki/Panac%C3%A9e)
