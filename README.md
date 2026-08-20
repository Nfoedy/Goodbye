# Goodbye

## 🌆 Mood and Game Intent

**Goodbye** vuole trasmettere la sensazione di un momento di cambiamento e di separazione.

Il trasloco rappresenta la conclusione di una fase: il giocatore raccoglie ciò che vuole portare con sé, carica il camion e infine lascia la città.

L'atmosfera è volutamente semplice e malinconica, accompagnata da una musica di sottofondo che richiama il tema dell'addio e della partenza.

La sequenza finale, nella quale il camion continua il proprio viaggio mentre la scena sfuma progressivamente verso il nero, è stata realizzata per rafforzare questa sensazione e concludere il gioco in modo coerente con il titolo **Goodbye**.

---

## 🎯 Project Goals

**Goodbye** è stato sviluppato durante il **Master in Computer Game Development** principalmente come progetto per i moduli di **Physics** e **High Level Programming**.

L'obiettivo principale non era quello di realizzare un gioco completo e rifinito in ogni suo aspetto, ma utilizzare un piccolo gameplay loop per mettere in pratica gli argomenti affrontati durante i corsi.

Per questo motivo lo sviluppo si è concentrato soprattutto su:

- Interazione fisica con gli oggetti;
- Physics Handle;
- Massa e comportamento degli oggetti;
- Collisioni e danni fisici;
- Chaos Vehicles;
- Gestione fisica del camion;
- Reflection e modifica di proprietà a runtime.

Aspetti come **UI, level design, grafica e audio** sono stati sviluppati solamente quanto necessario per rendere il progetto completo, comprensibile e giocabile, senza rappresentare il focus principale del lavoro.

L'obiettivo era quindi costruire un contesto di gameplay nel quale le feature tecniche sviluppate avessero una reale funzione, evitando di presentarle solamente come test o sistemi isolati.

---

## 🎮 Overview

In **Goodbye** il giocatore deve completare un trasloco prima dello scadere del tempo.

Gli oggetti presenti nella casa possono essere raccolti, trasportati e caricati fisicamente all'interno di un camion.

Ogni oggetto possiede un determinato valore in punti e il giocatore deve raggiungere il punteggio richiesto dalla difficoltà selezionata.

Una volta completato il carico, il giocatore può entrare nel camion e guidare verso l'uscita della città.

---

## 🕹️ Gameplay

Il giocatore dispone di un tempo limitato per raccogliere gli oggetti presenti nella casa.

Gli oggetti vengono caricati fisicamente nel cassone del camion e il loro punteggio viene conteggiato solamente quando si trovano realmente all'interno della **Cargo Zone**.

La partita può terminare in due modi.

### 🏆 Victory

La vittoria viene ottenuta quando il giocatore raggiunge la **Finish Zone**:

- entro il tempo disponibile;
- mentre sta guidando il camion;
- con il punteggio richiesto.

In caso di vittoria viene avviata una breve sequenza cinematografica finale che accompagna il camion fuori dalla città e conclude la partita.

### ❌ Defeat

La partita termina con una sconfitta quando:

- il tempo raggiunge zero;
- il giocatore raggiunge la Finish Zone senza il punteggio necessario.

In caso di sconfitta viene mostrata direttamente la schermata finale.

---

## ✨ Main Features

### Physics Grab System

Il giocatore può raccogliere e trasportare gli oggetti presenti nella casa attraverso un sistema basato su **Line Trace** e **Physics Handle**.

Gli oggetti continuano a utilizzare la simulazione fisica anche durante il trasporto, permettendo loro di reagire naturalmente agli urti e all'ambiente.

---

### Object Weight

Ogni oggetto possiede una massa fisica che influenza il sistema di Grab.

Gli oggetti più pesanti vengono trasportati più lentamente, mentre quelli che superano il peso massimo consentito non possono essere sollevati.

---

### Hand IK

Durante la raccolta degli oggetti viene utilizzato un sistema di **Two Bone IK**.

La mano destra del personaggio raggiunge il punto dell'oggetto selezionato prima che il Physics Handle completi la presa, rendendo l'interazione più naturale.

---

### Physical Damage and Health

Gli oggetti possono subire danni in seguito alle collisioni fisiche.

La quantità di danno dipende dalla forza dell'impatto e dalla massa dell'oggetto. Quando la salute raggiunge zero, l'oggetto viene distrutto.

Una Health Bar temporanea permette al giocatore di visualizzare lo stato dell'oggetto dopo un impatto.

---

### Cargo System

Il camion contiene una **Cargo Zone** che rileva automaticamente gli oggetti caricati nel cassone.

Il sistema mantiene aggiornati sia il numero di oggetti trasportati sia il punteggio complessivo, gestendo anche eventuali oggetti che vengono rimossi o distrutti.

---

### Chaos Vehicle

Il camion è stato realizzato utilizzando il sistema **Chaos Vehicles** di Unreal Engine.

Sono state configurate separatamente ruote anteriori e posteriori, trazione, sterzo, frenata, massa, motore e trasmissione per ottenere un veicolo completamente guidabile dal giocatore.

---

### Enter / Exit Vehicle

Il giocatore può entrare e uscire dal camion durante la partita.

Il sistema trasferisce dinamicamente il controllo tra il Character e il veicolo, permettendo di passare dal gameplay in prima persona alla guida del Cargo.

---

### Automatic Parking

Dopo l'uscita del giocatore, il camion continua inizialmente a muoversi utilizzando la propria inerzia.

Quando il veicolo rimane fermo per un breve periodo, viene automaticamente parcheggiato per evitare movimenti indesiderati causati dalle interazioni fisiche.

---

### Victory Sequence

Quando vengono soddisfatte le condizioni di vittoria, il camion continua automaticamente lungo la strada mentre viene riprodotta una **Level Sequence** dedicata.

La sequenza termina con un fade e con la schermata conclusiva del gioco.

---

### Difficulty System

Il gioco dispone di tre difficoltà:

| Difficulty | Required Score |
|------------|---------------:|
| Easy       | 5              |
| Normal     | 8              |
| Hard       | 12             |

La difficoltà viene selezionata dal Main Menu e determina il punteggio necessario per completare il trasloco.

---

### User Interface

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

### Audio

Sono presenti effetti audio per le principali interazioni del gioco, tra cui Grab, Drop, distruzione degli oggetti, accensione e spegnimento del camion e pulsanti della UI.

Il gameplay è inoltre accompagnato da una musica di sottofondo.


---
## 🧠 High Level Programming Assignment

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

---

## 📌 Final Considerations

Lo sviluppo di **Goodbye** mi ha permesso di applicare all'interno dello stesso progetto diversi sistemi studiati durante il Master.

La parte più importante del lavoro è stata riuscire a trasformare concetti prevalentemente tecnici in vere meccaniche di gameplay.

Il sistema di Grab, il comportamento degli oggetti, i danni provocati dalle collisioni e il camion basato su Chaos Vehicles costituiscono la parte principale del lavoro svolto per **Physics**.

Allo stesso modo, il Cargo Speed PowerUp mi ha permesso di integrare la **Reflection** all'interno di una situazione concreta di gameplay per la consegna di **High Level Programming**.

Il risultato finale è volutamente un progetto di dimensioni contenute, ma possiede un gameplay loop completo e mi ha permesso di approfondire diversi aspetti della programmazione C++ e di Unreal Engine.

Il progetto è stato infine testato sia all'interno dell'Unreal Editor sia attraverso una **Windows packaged build**.

---

## 🔮 Future Developments

Il progetto è stato sviluppato principalmente come esercizio tecnico e didattico, quindi diversi aspetti potrebbero essere ulteriormente approfonditi in una versione futura.

Alcuni possibili sviluppi sono:

- miglioramento della UI e della presentazione generale;
- maggiore varietà di oggetti trasportabili;
- level design più articolato;
- ulteriori interazioni fisiche con l'ambiente;
- miglioramento degli effetti audio e visivi;
- animazioni del Character più rifinite;
- miglioramento della guida e della configurazione del veicolo;
- introduzione di nuove situazioni di gameplay basate sulla fisica.

Questi elementi non sono stati approfonditi nella versione attuale perché avrebbero spostato il focus dagli obiettivi principali del progetto: **Physics** e **High Level Programming**.

---

## 📸 Video & Media


