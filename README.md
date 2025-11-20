# area_matching

## Context

Open Maps For Europe 2 est un projet qui a pour objectif de développer un nouveau processus de production dont la finalité est la construction d'un référentiel cartographique pan-européen à grande échelle (1:10 000).

L'élaboration de la chaîne de production a nécessité le développement d'un ensemble de composants logiciels qui constituent le projet [OME2](https://github.com/openmapsforeurope2/OME2).


## Description

Le présent outil est dédié à la mise en cohérence de classes d'objets surfaciques de deux pays autour de leur(s) frontière(s) commune(s).

Lorsqu'elle est lancée l'application traite un couple de pays frontaliers. Pour raccorder l'ensemble des surfaces d'un pays le programme doit être lancé successivement sur ses différentes frontières (en considérant l'ensemble de ses pays limitrophes).

## Fonctionnement

Le programme ne manipule pas directement les données de production. Les données à traiter, localisées autour de la frontière, sont extraites dans une table de travail. A l'issu du traitement les données dérivées sont injectées dans la table source en remplacement des données initiales.

Le processus de mise en cohérence est décomposé en plusieurs étapes. Un numéro est attribué à chaque étape. Une table de travail préfixée de ce numéro est délivrée en sortie de chaque étape. Chaque étape prend en données d'entrées les tables de travail générées lors d'étapes antérieures.

Voici l'ensemble des étapes constitutives du processus de raccordement:

**410** - découpe des surfaces des deux pays avec les frontières
<br>
**420** - pour les surfaces hors pays, on soustrait les portions de surface superposées aux surfaces du pays hôte.
<br>
**425** - suppression des surfaces hors pays isolées (non connectés directement ou indirectement avec une surface dans le pays)
<br>
**430** - fusion des surfaces en fonction du pays d'origine, de l'identifiant et le la longeur de zone de contact.

## Configuration

L'outil s'appuie sur de nombreux paramètres de configuration permettant d'adapter le comportement des algorithmes en fonctions des spécificités nationales (sémantique, précision, échelle, conventions de modélisation...).

On trouve dans le [dossier de configuration](https://github.com/openmapsforeurope2/area_matching/tree/main/config) les fichiers suivants :

- epg_parameters.ini : regroupe des paramètres de base issus de la bibliothèque libepg qui constitue le socle de développement l'outil. Ce fichier est aussi le fichier chapeau qui pointe vers les autres fichiers de configurations.
- db_conf.ini : informations de connexion à la base de données.
- theme_parameters_drainage_basin.ini : configuration des paramètres spécifiques à l'application pour la classe d'objet _drainage_basin_.
- theme_parameters_glacier_snowfield.ini : configuration des paramètres spécifiques à l'application pour la classe d'objet _glacier_snowfield_.

## Utilisation

L'outil s'utilise en ligne de commande.

Paramètres:
* c [obligatoire] : chemin vers le fichier de configuration
* s [obligatoire] : suffix de la table de travail
* t [obligatoire] : nom de la classe d'objet (drainage_basin ou glacier_snowfield)
* sp [optionnel] : étape(s) à executer (exemples: 410 ; 410,425 ; 410-425)
* arguments libres [obligatoire] : codes des deux pays frontaliers

<br>

Exemple d'appel pour lancer successivement l'ensemble des étapes sur la frontière franco-belge :
~~~
bin/area_matching --c path/to/config/epg_parameters.ini --t drainage_basin --s 20251113 fr be
~~~

Exemple d'appel pour ne lancer qu'une seule étape :
~~~
bin/area_matching --c path/to/config/epg_parameters.ini --t drainage_basin --s 20251113 --sp 420 fr be
~~~
