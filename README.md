# Goodbye

**Goodbye** è un piccolo gioco sviluppato in **Unreal Engine 5.8** utilizzando principalmente **C++**.

Il progetto è stato realizzato durante il **Master in Computer Game Development** come progetto per i moduli di **Physics** e **High Level Programming**.

L'obiettivo è stato quello di applicare gli argomenti studiati durante i corsi all'interno di un gameplay completo, invece di realizzare solamente test tecnici separati.

---

## Overview

In **Goodbye** il giocatore deve completare un trasloco prima dello scadere del tempo.

Gli oggetti presenti nella casa possono essere raccolti, trasportati e caricati fisicamente all'interno di un camion.

Ogni oggetto possiede un determinato valore in punti e il giocatore deve raggiungere il punteggio richiesto dalla difficoltà selezionata.

Una volta completato il carico, il giocatore può entrare nel camion e guidare verso l'uscita della città.

---

# Gameplay

Il giocatore dispone di un tempo limitato per raccogliere gli oggetti presenti nella casa.

Gli oggetti vengono caricati fisicamente nel cassone del camion e il loro punteggio viene conteggiato solamente quando si trovano realmente all'interno della **Cargo Zone**.

La partita può terminare in due modi.

## Victory

La vittoria viene ottenuta quando il giocatore raggiunge la **Finish Zone**:

- entro il tempo disponibile;
- mentre sta guidando il camion;
- con il punteggio richiesto.

In caso di vittoria viene avviata una breve sequenza cinematografica finale che accompagna il camion fuori dalla città e conclude la partita.

## Defeat

La partita termina con una sconfitta quando:

- il tempo raggiunge zero;
- il giocatore raggiunge la Finish Zone senza il punteggio necessario.

In caso di sconfitta viene mostrata direttamente la schermata finale.

---

# Main Features

## Physics Grab System

Il giocatore può raccogliere e trasportare gli oggetti presenti nella casa attraverso un sistema basato su **Line Trace** e **Physics Handle**.

Gli oggetti continuano a utilizzare la simulazione fisica anche durante il trasporto, permettendo loro di reagire naturalmente agli urti e all'ambiente.

---

## Object Weight

Ogni oggetto possiede una massa fisica che influenza il sistema di Grab.

Gli oggetti più pesanti vengono trasportati più lentamente, mentre quelli che superano il peso massimo consentito non possono essere sollevati.

---

## Hand IK

Durante la raccolta degli oggetti viene utilizzato un sistema di **Two Bone IK**.

La mano destra del personaggio raggiunge il punto dell'oggetto selezionato prima che il Physics Handle completi la presa, rendendo l'interazione più naturale.

---

## Physical Damage and Health

Gli oggetti possono subire danni in seguito alle collisioni fisiche.

La quantità di danno dipende dalla forza dell'impatto e dalla massa dell'oggetto. Quando la salute raggiunge zero, l'oggetto viene distrutto.

Una Health Bar temporanea permette al giocatore di visualizzare lo stato dell'oggetto dopo un impatto.

---

## Cargo System

Il camion contiene una **Cargo Zone** che rileva automaticamente gli oggetti caricati nel cassone.

Il sistema mantiene aggiornati sia il numero di oggetti trasportati sia il punteggio complessivo, gestendo anche eventuali oggetti che vengono rimossi o distrutti.

---

## Chaos Vehicle

Il camion è stato realizzato utilizzando il sistema **Chaos Vehicles** di Unreal Engine.

Sono state configurate separatamente ruote anteriori e posteriori, trazione, sterzo, frenata, massa, motore e trasmissione per ottenere un veicolo completamente guidabile dal giocatore.

---

## Enter / Exit Vehicle

Il giocatore può entrare e uscire dal camion durante la partita.

Il sistema trasferisce dinamicamente il controllo tra il Character e il veicolo, permettendo di passare dal gameplay in prima persona alla guida del Cargo.

---

## Automatic Parking

Dopo l'uscita del giocatore, il camion continua inizialmente a muoversi utilizzando la propria inerzia.

Quando il veicolo rimane fermo per un breve periodo, viene automaticamente parcheggiato per evitare movimenti indesiderati causati dalle interazioni fisiche.

---

## Victory Sequence

Quando vengono soddisfatte le condizioni di vittoria, il camion continua automaticamente lungo la strada mentre viene riprodotta una **Level Sequence** dedicata.

La sequenza termina con un fade e con la schermata conclusiva del gioco.

---

## Difficulty System

Il gioco dispone di tre difficoltà:

| Difficulty | Required Score |
|------------|---------------:|
| Easy       | 5              |
| Normal     | 8              |
| Hard       | 12             |

La difficoltà viene selezionata dal Main Menu e determina il punteggio necessario per completare il trasloco.

---

## User Interface

Il progetto comprende:

- Main Menu;
- selezione della difficoltà;
- HUD con timer e punteggio;
- Health Bar degli oggetti;
- Pause Menu;
- schermate di Victory e Defeat;
- Play Again;
- ritorno al Main Menu.

---

## Audio

Sono presenti effetti audio per le principali interazioni del gioco, tra cui Grab, Drop, distruzione degli oggetti, accensione e spegnimento del camion e pulsanti della UI.

Il gameplay è inoltre accompagnato da una musica di sottofondo.


---
# High Level Programming Assignment

Una parte di **Goodbye** è stata sviluppata specificamente per la consegna del modulo di **High Level Programming**.

L'obiettivo della consegna era utilizzare la **Reflection** per modificare una proprietà di un Actor durante l'esecuzione, individuandola attraverso il suo nome invece di accedere direttamente alla variabile.

Per applicare questo concetto all'interno del gameplay ho realizzato un **Cargo Speed PowerUp**.

---

## Cargo Speed PowerUp

Il PowerUp può essere raccolto mentre il giocatore sta guidando il camion.

Quando viene raccolto, aumenta del **15% la potenza del motore**.

La particolarità del sistema è che il PowerUp non modifica direttamente la variabile del camion, ma contiene il nome della proprietà che deve essere modificata.

La Reflection viene quindi utilizzata per cercare quella proprietà a runtime e applicare il moltiplicatore configurato.

Nel progetto il PowerUp modifica:

```text
EnginePowerMultiplier
```

portandolo da:

```text
1.00
```

a:

```text
1.15
```

La nuova potenza viene poi applicata al motore del camion.

---

## Why Reflection?

Lo stesso risultato avrebbe potuto essere ottenuto modificando direttamente la variabile del veicolo.

La consegna richiedeva però di utilizzare la **Reflection**, quindi ho scelto di integrare questo concetto all'interno di una vera feature di gameplay.

In questo modo il PowerUp conosce solamente:

```text
- l'oggetto da modificare;
- il nome della proprietà;
- il moltiplicatore da applicare.
```

La proprietà reale viene individuata solamente durante l'esecuzione.

Questo mi ha permesso di utilizzare la Reflection in modo pratico, mantenendo la feature integrata nel gameplay di **Goodbye** invece di realizzare un semplice test separato.