# Cartographie
## Kernel
- **Plages physiques** :
  - 0x00000000 - 0x003FFFFF : Identité mappée pour le code et les données kernel.
- **Plages virtuelles** :
  - 0x00000000 - 0x003FFFFF : Identité mappée pour le code et les données kernel.



### Configuration des tables de pages

#### Kernel PGD
- **PGD** : 0x350000
- **Page Table** : 0x351000

#### Task 1 PGD
- **PGD** : 0x352000
- **Page Tables** :
  - 0x353000 : Identité mappée (kernel).
  - 0x354000 : Mémoire utilisateur (0x00400000 - 0x007FFFFF).
  - 0x355000 : Mémoire partagée (0x00E00000).

#### Task 2 PGD
- **PGD** : 0x356000
- **Page Tables** :
  - 0x357000 : Identité mappée (kernel).
  - 0x358000 : Mémoire utilisateur (0x00800000 - 0x00BFFFFF).
  - 0x359000 : Mémoire partagée (0x00E00000).

### Stack TSS

| Tâche        | Adresse pile  TSS  | Plage |
|--------------|---------------------------|-----------------------------|
| Task 1       | 0x380FFF       | 0x380000 - 0x380FFF         |
| Task 2       | 0x390FFF       | 0x390000 - 0x390FFF         |

## Task 1
- **Plages physiques** :
  - 0x00400000 - 0x007FFFFF : Mémoire utilisateur pour la tâche 1.
  - 0x00E00000 : Adresse utilisée pour une page unique de mémoire partagée.
- **Plages virtuelles** :
  - 0x00400000 - 0x007FFFFF : Mappée pour la mémoire utilisateur.
  - 0x00800000 : Mappée vers la mémoire partagée.
- **Contenu de 0x00400000 - 0x007FFFFF** :
  - Code : Fonction `task1_function`.
  - Stack .

## Task 2
- **Plages physiques** :
  - 0x00800000 - 0x00BFFFFF : Mémoire utilisateur pour la tâche 2.
  - 0x00E00000 : Adresse utilisée pour une page unique de mémoire partagée.
- **Plages virtuelles** :
  - 0x00800000 - 0x00BFFFFF : Mappée pour la mémoire utilisateur.
  - 0x00400000 : Mappée vers la mémoire partagée.
- **Contenu de 0x00800000 - 0x00BFFFFF** :
  - Code : Fonctions `task2_function` et `sys_counter` .
  - Stack .


