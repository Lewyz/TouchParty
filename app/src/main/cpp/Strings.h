#ifndef TOUCHPARTY_STRINGS_H
#define TOUCHPARTY_STRINGS_H

#include <string>

enum class Language {
    SPANISH,
    ENGLISH
};

enum class StringId {
    TITLE_GAME,
    SERVER_CONNECTED,
    SERVER_DISCONNECTED,
    TEST_1V1,
    NICKNAME_LABEL,
    NICKNAME_EDIT,
    SEARCH_ROOMS,
    CREATE_ROOM,
    CREATE_NEW_ROOM_TITLE,
    ROOM_NAME_LABEL,
    MAX_PLAYERS_NOTE,
    LOBBY_REQUIRED_PLAYERS,
    LOBBY_START_MATCH,
    LOBBY_ADD_TEST_PLAYER,
    AVAILABLE_ROOMS_TITLE,
    FILTER_ALL,
    FILTER_PUBLIC,
    FILTER_PRIVATE,
    SCROLL_UP,
    SCROLL_DOWN,
    EMPTY_ROOMS_LINE1,
    EMPTY_ROOMS_LINE2,
    JOIN_ROOM,
    REFRESH,
    BACK,
    RED_WINS,
    BLUE_WINS,
    DRAW,
    PLAY_AGAIN,
    EXIT,
    COUNTDOWN_GO,
    JOIN_NOTIFICATION_SUFFIX,
    PROMPT_ENTER_NICKNAME,
    PROMPT_ROOM_NAME,
    PROMPT_ROOM_PIN,
    LANGUAGE_LABEL,
    LANGUAGE_SPANISH,
    LANGUAGE_ENGLISH,
    LANGUAGE_SELECT,
    SAVE_CONTINUE,
    WELCOME_BACK,
    NICKNAME_TAP_HINT,
    WELCOME_SETUP_TITLE,
    PLAYER_JOINED,
    PLAYER_LEFT,
    NOW_OWNER,
    MATCH_OVER_REASON,
    TYPE_LABEL,
    PUBLIC_LABEL,
    PRIVATE_LABEL,
    PIN_LABEL,
    PIN_MASK,
    PLAYERS_COUNT,
    MATCH_IN_PROGRESS,
    WAITING_CREATOR,
    WAITING_LEADER,
    RECONNECTING,
    YOU_WON,
    YOU_LOST,
    TIE_GAME,
    SETTINGS,
    REQUIRED_2_PLAYERS,
    LEAVE_CONFIRM_QUESTION,
    YES_LEAVE,
    CANCEL,
    WELCOME_BACK_NOTIF,
    PLAYER_JOINED_NOTIF,
    SEARCH_PREFIX,
    PRIVATE_WITH_PIN,
    PIN_LABEL_WORD,
    PRIVATE_ROOM_PIN_PROMPT,
    SERVER_BROWSER,
    LOBBIES_HEADER,
    SEARCH_LABEL,
    FULL_BADGE,
    NEED_OPPOSING_TEAMS
};

class Strings {
public:
    static void setLanguage(Language lang) {
        currentLang_ = lang;
    }

    static Language getLanguage() {
        return currentLang_;
    }

    static std::string get(StringId id) {
        return get(id, currentLang_);
    }

    static std::string get(StringId id, Language lang) {
        switch (id) {
            case StringId::TITLE_GAME:
                return "TOUCHPARTY CUBE ARENA";
            case StringId::SERVER_CONNECTED:
                return (lang == Language::SPANISH) ? "SERVIDOR: CONECTADO" : "SERVER: CONNECTED";
            case StringId::SERVER_DISCONNECTED:
                return (lang == Language::SPANISH) ? "SERVIDOR: DESCONECTADO (MODO LOCAL)" : "SERVER: DISCONNECTED (LOCAL MODE)";
            case StringId::TEST_1V1:
                return "TEST 1V1";
            case StringId::NICKNAME_LABEL:
                return "NICKNAME: ";
            case StringId::NICKNAME_EDIT:
                return (lang == Language::SPANISH) ? " (EDITAR)" : " (EDIT)";
            case StringId::SEARCH_ROOMS:
                return (lang == Language::SPANISH) ? "BUSCAR SALAS" : "SEARCH ROOMS";
            case StringId::CREATE_ROOM:
                return (lang == Language::SPANISH) ? "CREAR SALA" : "CREATE ROOM";
            case StringId::CREATE_NEW_ROOM_TITLE:
                return (lang == Language::SPANISH) ? "CREAR NUEVA SALA" : "CREATE NEW ROOM";
            case StringId::ROOM_NAME_LABEL:
                return (lang == Language::SPANISH) ? "NOMBRE: " : "NAME: ";
            case StringId::MAX_PLAYERS_NOTE:
                return (lang == Language::SPANISH) ? "MAX 8 JUGADORES - 90 SEG" : "MAX 8 PLAYERS - 90 SEC";
            case StringId::LOBBY_REQUIRED_PLAYERS:
                return (lang == Language::SPANISH) ? "REQUERIDOS 2 JUGADORES (1/8)" : "REQUIRED 2 PLAYERS (1/8)";
            case StringId::LOBBY_START_MATCH:
                return (lang == Language::SPANISH) ? "INICIAR PARTIDA" : "START MATCH";
            case StringId::LOBBY_ADD_TEST_PLAYER:
                return (lang == Language::SPANISH) ? "AGREGAR JUGADOR TEST (+1)" : "ADD TEST PLAYER (+1)";
            case StringId::AVAILABLE_ROOMS_TITLE:
                return (lang == Language::SPANISH) ? "SALAS DISPONIBLES" : "AVAILABLE ROOMS";
            case StringId::FILTER_ALL:
                return (lang == Language::SPANISH) ? "TODAS" : "ALL";
            case StringId::FILTER_PUBLIC:
                return (lang == Language::SPANISH) ? "PUBLICAS" : "PUBLIC";
            case StringId::FILTER_PRIVATE:
                return (lang == Language::SPANISH) ? "PRIVADAS" : "PRIVATE";
            case StringId::SCROLL_UP:
                return (lang == Language::SPANISH) ? "ARRIBA" : "UP";
            case StringId::SCROLL_DOWN:
                return (lang == Language::SPANISH) ? "ABAJO" : "DOWN";
            case StringId::EMPTY_ROOMS_LINE1:
                return (lang == Language::SPANISH) ? "NO HAY SALAS DISPONIBLES EN ESTE MOMENTO" : "NO ROOMS AVAILABLE AT THIS TIME";
            case StringId::EMPTY_ROOMS_LINE2:
                return (lang == Language::SPANISH) ? "PULSA CREAR SALA PARA INICIAR UNA NUEVA" : "PRESS CREATE ROOM TO START A NEW ONE";
            case StringId::JOIN_ROOM:
                return (lang == Language::SPANISH) ? "UNIRSE" : "JOIN";
            case StringId::REFRESH:
                return (lang == Language::SPANISH) ? "ACTUALIZAR" : "REFRESH";
            case StringId::BACK:
                return (lang == Language::SPANISH) ? "VOLVER" : "BACK";
            case StringId::RED_WINS:
                return (lang == Language::SPANISH) ? "¡VICTORIA ROJA!" : "RED WINS!";
            case StringId::BLUE_WINS:
                return (lang == Language::SPANISH) ? "¡VICTORIA AZUL!" : "BLUE WINS!";
            case StringId::DRAW:
                return (lang == Language::SPANISH) ? "¡EMPATE!" : "DRAW!";
            case StringId::PLAY_AGAIN:
                return (lang == Language::SPANISH) ? "OTRA VEZ" : "PLAY AGAIN";
            case StringId::EXIT:
                return (lang == Language::SPANISH) ? "SALIR" : "EXIT";
            case StringId::COUNTDOWN_GO:
                return (lang == Language::SPANISH) ? "¡VAMOS!" : "GO!";
            case StringId::JOIN_NOTIFICATION_SUFFIX:
                return (lang == Language::SPANISH) ? " SE HA UNIDO A LA SALA!" : " HAS JOINED THE ROOM!";
            case StringId::PROMPT_ENTER_NICKNAME:
                return (lang == Language::SPANISH) ? "INGRESA TU NICKNAME DE JUGADOR" : "ENTER YOUR PLAYER NICKNAME";
            case StringId::PROMPT_ROOM_NAME:
                return (lang == Language::SPANISH) ? "NOMBRE DE LA SALA" : "ROOM NAME";
            case StringId::PROMPT_ROOM_PIN:
                return (lang == Language::SPANISH) ? "PIN DE LA SALA (4 DIGITOS)" : "ROOM PIN (4 DIGITS)";
            case StringId::LANGUAGE_LABEL:
                return (lang == Language::SPANISH) ? "IDIOMA:" : "LANGUAGE:";
            case StringId::LANGUAGE_SPANISH:
                return "SPANISH";
            case StringId::LANGUAGE_ENGLISH:
                return "ENGLISH";
            case StringId::LANGUAGE_SELECT:
                return (lang == Language::SPANISH) ? "SELECCIONA TU IDIOMA" : "SELECT YOUR LANGUAGE";
            case StringId::SAVE_CONTINUE:
                return (lang == Language::SPANISH) ? "GUARDAR Y CONTINUAR" : "SAVE & CONTINUE";
            case StringId::WELCOME_BACK:
                return (lang == Language::SPANISH) ? "¡BIENVENIDO DE NUEVO!" : "WELCOME BACK!";
            case StringId::NICKNAME_TAP_HINT:
                return (lang == Language::SPANISH) ? "TOCA PARA INGRESAR TU NICKNAME" : "TAP TO ENTER YOUR NICKNAME";
            case StringId::WELCOME_SETUP_TITLE:
                return (lang == Language::SPANISH) ? "BIENVENIDO A TOUCHPARTY" : "WELCOME TO TOUCHPARTY";
            case StringId::PLAYER_JOINED:
                return (lang == Language::SPANISH) ? "¡UN JUGADOR SE HA UNIDO A LA SALA!" : "A PLAYER HAS JOINED THE ROOM!";
            case StringId::PLAYER_LEFT:
                return (lang == Language::SPANISH) ? "¡UN JUGADOR HA SALIDO DE LA SALA!" : "A PLAYER HAS LEFT THE ROOM!";
            case StringId::NOW_OWNER:
                return (lang == Language::SPANISH) ? "¡AHORA ERES EL CREADOR DE LA SALA!" : "YOU ARE NOW THE ROOM CREATOR!";
            case StringId::MATCH_OVER_REASON:
                return (lang == Language::SPANISH) ? "PARTIDA FINALIZADA: " : "MATCH OVER: ";
            case StringId::TYPE_LABEL:
                return (lang == Language::SPANISH) ? "TIPO: " : "TYPE: ";
            case StringId::PUBLIC_LABEL:
                return (lang == Language::SPANISH) ? "PUBLICA" : "PUBLIC";
            case StringId::PRIVATE_LABEL:
                return (lang == Language::SPANISH) ? "PRIVADA" : "PRIVATE";
            case StringId::PIN_LABEL:
                return "PIN: ";
            case StringId::PIN_MASK:
                return "PIN: ----";
            case StringId::PLAYERS_COUNT:
                return (lang == Language::SPANISH) ? "JUGADORES" : "PLAYERS";
            case StringId::MATCH_IN_PROGRESS:
                return (lang == Language::SPANISH) ? "PARTIDA EN CURSO" : "MATCH IN PROGRESS";
            case StringId::WAITING_CREATOR:
                return (lang == Language::SPANISH) ? "ESPERANDO AL CREADOR..." : "WAITING FOR CREATOR...";
            case StringId::WAITING_LEADER:
                return (lang == Language::SPANISH) ? "ESPERANDO AL LIDER..." : "WAITING FOR LEADER...";
            case StringId::RECONNECTING:
                return (lang == Language::SPANISH) ? "RECONECTANDO A LA PARTIDA..." : "RECONNECTING TO MATCH...";
            case StringId::YOU_WON:
                return (lang == Language::SPANISH) ? "¡HAS GANADO!" : "YOU WON!";
            case StringId::YOU_LOST:
                return (lang == Language::SPANISH) ? "HAS PERDIDO" : "YOU LOST";
            case StringId::TIE_GAME:
                return (lang == Language::SPANISH) ? "¡EMPATE!" : "TIE GAME!";
            case StringId::SETTINGS:
                return (lang == Language::SPANISH) ? "CONFIGURACION" : "SETTINGS";
            case StringId::REQUIRED_2_PLAYERS:
                return (lang == Language::SPANISH) ? "REQUERIDOS 2 JUGADORES" : "REQUIRED 2 PLAYERS";
            case StringId::LEAVE_CONFIRM_QUESTION:
                return (lang == Language::SPANISH) ? "¿DESEAS SALIR DE LA SALA?" : "DO YOU WANT TO LEAVE THE ROOM?";
            case StringId::YES_LEAVE:
                return (lang == Language::SPANISH) ? "SI, SALIR" : "YES, LEAVE";
            case StringId::CANCEL:
                return (lang == Language::SPANISH) ? "CANCELAR" : "CANCEL";
            case StringId::WELCOME_BACK_NOTIF:
                return (lang == Language::SPANISH) ? "¡BIENVENIDO DE NUEVO, @!" : "WELCOME BACK, @!";
            case StringId::PLAYER_JOINED_NOTIF:
                return (lang == Language::SPANISH) ? "¡@ SE HA UNIDO A LA SALA!" : "@ HAS JOINED THE ROOM!";
            case StringId::SEARCH_PREFIX:
                return (lang == Language::SPANISH) ? "BUSCAR: " : "SEARCH: ";
            case StringId::PRIVATE_WITH_PIN:
                return (lang == Language::SPANISH) ? "PRIVADA (PIN: @)" : "PRIVATE (PIN: @)";
            case StringId::PIN_LABEL_WORD:
                return "PIN";
            case StringId::PRIVATE_ROOM_PIN_PROMPT:
                return (lang == Language::SPANISH) ? "PIN DE SALA PRIVADA (@" : "PRIVATE ROOM PIN (@";
            case StringId::SERVER_BROWSER:
                return (lang == Language::SPANISH) ? "BUSCADOR DE SALAS" : "SERVER BROWSER";
            case StringId::LOBBIES_HEADER:
                return (lang == Language::SPANISH) ? "SALAS" : "LOBBIES";
            case StringId::SEARCH_LABEL:
                return (lang == Language::SPANISH) ? "BUSCAR" : "SEARCH";
            case StringId::FULL_BADGE:
                return (lang == Language::SPANISH) ? "[LLENO]" : "[FULL]";
            case StringId::NEED_OPPOSING_TEAMS:
                return (lang == Language::SPANISH) ? "REQUERIDOS EN AMBOS EQUIPOS" : "NEED PLAYERS IN BOTH TEAMS";
        }
        return "";
    }

private:
    static inline Language currentLang_ = Language::SPANISH;
};

#endif // TOUCHPARTY_STRINGS_H