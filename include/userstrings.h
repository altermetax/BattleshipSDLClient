#pragma once

// 0 English - 1 Italian
#define LANGUAGE 1

#if(LANGUAGE == 0)
#define CONNECTING_MSG "Connecting to %s..."
#define CONNECTED_MSG "Connected. Waiting for a match..."
#define PLACE_SHIPS_MSG "Place your ships by moving the mouse on your field."
#define PLACE_SHIPS_ONGRID_MSG "Place your ships. Mouse left: place, mouse right: rotate, mouse middle: undo, mouse wheel: cycle through."
#define WAIT_SHIPS_MSG "Waiting for opponent to finish placing their ships..."
#define ATTACK_MSG "It's your turn. Attack by moving the mouse on the opponent's field."
#define ATTACK_ONGRID_MSG "Click to attack %c%d"
#define WAIT_TURN_MSG "It's your opponent's turn."
#define YOU_WIN_MSG "You win!"
#define YOU_LOSE_MSG "You lose"
#endif

#if(LANGUAGE == 1)
#define CONNECTING_MSG "Connessione a %s..."
#define CONNECTED_MSG "Connesso. In attesa di un match..."
#define PLACE_SHIPS_MSG "Posiziona le navi spostando il mouse sul tuo campo."
#define PLACE_SHIPS_ONGRID_MSG "Posiziona le navi. Tasto sinistro: posiziona, tasto destro: ruota, tasto centrale: annulla azione, rotella: scorri le navi."
#define WAIT_SHIPS_MSG "Attendi che l'avversario finisca di posizionare le proprie navi..."
#define ATTACK_MSG "E' il tuo turno. Attacca spostando il mouse sul campo avversario."
#define ATTACK_ONGRID_MSG "Clicca per attaccare %c%d"
#define WAIT_TURN_MSG "E' il turno del tuo avversario."
#define YOU_WIN_MSG "Hai vinto!"
#define YOU_LOSE_MSG "Hai perso"
#endif