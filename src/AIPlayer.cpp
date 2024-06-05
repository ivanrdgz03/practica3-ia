#include "AIPlayer.h"
#include "Parchis.h"

using namespace std;

const double masinf = 9999999999.0, menosinf = -9999999999.0;
const double gana = masinf - 1, pierde = menosinf + 1;
const int num_pieces = 3;
const int PROFUNDIDAD_MINIMAX = 4;  // Umbral maximo de profundidad para el metodo MiniMax
const int PROFUNDIDAD_ALFABETA = 6; // Umbral maximo de profundidad para la poda Alfa_Beta

bool AIPlayer::move()
{
    cout << "Realizo un movimiento automatico" << endl;

    color c_piece;
    int id_piece;
    int dice;
    think(c_piece, id_piece, dice);

    cout << "Movimiento elegido: " << str(c_piece) << " " << id_piece << " " << dice << endl;

    actual->movePiece(c_piece, id_piece, dice);
    return true;
}

void AIPlayer::thinkAleatorio(color &c_piece, int &id_piece, int &dice) const
{
    int player = actual->getCurrentPlayerId();
    vector<int> current_dices;
    vector<tuple<color, int>> current_pieces;
    current_dices = actual->getAvailableNormalDices(player);

    dice = current_dices[rand() % current_dices.size()];

    current_pieces = actual->getAvailablePieces(player, dice);

    if (current_pieces.size() > 0)
    {
        int random_id = rand() % current_pieces.size();
        id_piece = get<1>(current_pieces[random_id]);
        c_piece = get<0>(current_pieces[random_id]);
    }
    else
    {
        id_piece = SKIP_TURN;
        c_piece = actual->getCurrentColor();
    }
}

void AIPlayer::thinkAleatorioMasInteligente(color &c_piece, int &id_piece, int &dice) const
{
    int player = actual->getCurrentPlayerId();
    vector<int> current_dices;
    vector<tuple<color, int>> current_pieces;
    current_dices = actual->getAvailableNormalDices(player);
    vector<int> current_dices_que_pueden_mover_ficha;

    for (int i = 0; i < current_dices.size(); i++)
    {
        current_pieces = actual->getAvailablePieces(player, current_dices[i]);
        if (current_pieces.size() > 0)
            current_dices_que_pueden_mover_ficha.push_back(current_dices[i]);
    }
    if (current_dices_que_pueden_mover_ficha.size() == 0)
    {
        dice = current_dices[rand() % current_dices.size()];
        id_piece = SKIP_TURN;
        c_piece = actual->getCurrentColor();
    }
    else
    {
        dice = current_dices_que_pueden_mover_ficha[rand() % current_dices_que_pueden_mover_ficha.size()];
        current_pieces = actual->getAvailablePieces(player, dice);
        int random_id = rand() % current_pieces.size();
        id_piece = get<1>(current_pieces[random_id]);
        c_piece = get<0>(current_pieces[random_id]);
    }
}

void AIPlayer::thinkFichaMasAdelantada(color &c_piece, int &id_piece, int &dice) const
{
    thinkAleatorioMasInteligente(c_piece, id_piece, dice);
    int player = actual->getCurrentPlayerId();
    vector<tuple<color, int>> current_pieces = actual->getAvailablePieces(player, dice);
    int id_ficha_mas_adelantada = -1;
    color col_ficha_mas_adelantada = none;
    int min_distancia_meta = 9999;
    for (int i = 0; i < current_pieces.size(); i++)
    {
        color col = get<0>(current_pieces[i]);
        int id = get<1>(current_pieces[i]);
        int distancia_meta = actual->distanceToGoal(col, id);
        if (distancia_meta < min_distancia_meta)
        {
            min_distancia_meta = distancia_meta;
            id_ficha_mas_adelantada = id;
            col_ficha_mas_adelantada = col;
        }
    }
    if (id_ficha_mas_adelantada == -1)
    {
        id_piece = SKIP_TURN;
        c_piece = actual->getCurrentColor();
    }
    else
    {
        id_piece = id_ficha_mas_adelantada;
        c_piece = col_ficha_mas_adelantada;
    }
}

void AIPlayer::thinkMejorOpcion(color &c_piece, int &id_piece, int &dice) const
{
    ParchisBros hijos = actual->getChildren();
    bool me_quedo_con_esta_accion = false;
    int current_power = actual->getPowerBar(this->jugador).getPower();
    int max_power = -101;
    for (ParchisBros::Iterator it = hijos.begin();
         it != hijos.end() and !me_quedo_con_esta_accion; ++it)
    {
        Parchis siguiente_hijo = *it;
        if (siguiente_hijo.isEatingMove() or
            siguiente_hijo.isGoalMove() or
            (siguiente_hijo.gameOver() and siguiente_hijo.getWinner() == this->jugador))
        {
            me_quedo_con_esta_accion = true;
            c_piece = it.getMovedColor();
            id_piece = it.getMovedPieceId();
            dice = it.getMovedDiceValue();
        }
        else
        {
            int new_power = siguiente_hijo.getPower(this->jugador);
            if (new_power - current_power > max_power)
            {
                c_piece = it.getMovedColor();
                id_piece = it.getMovedPieceId();
                dice = it.getMovedDiceValue();
                max_power = new_power - current_power;
            }
        }
    }
}

double AIPlayer::thinkAlfaBeta(color &c_piece, int &id_piece, int &dice, double (*heuristic)(const Parchis &, int)) const
{
    double alpha = menosinf;
    double beta = masinf;
    const int PROFUNDIDAD_INICIAL = 0;
    return Poda_AlfaBeta(*actual, this->jugador, PROFUNDIDAD_INICIAL, PROFUNDIDAD_ALFABETA, c_piece, id_piece, dice, alpha, beta, heuristic);
}

double AIPlayer::Poda_AlfaBeta(const Parchis &actual, const int &jugador, const int &profundidad, const int &profundidad_max, color &c_piece, int &id_piece, int &dice, double alpha, double beta, double (*heuristic)(const Parchis &, int)) const
{
    if (profundidad == profundidad_max || actual.gameOver())
        return heuristic(actual, jugador);

    double salida = menosinf;

    ParchisBros hijos = actual.getChildren();

    if (jugador == actual.getCurrentPlayerId())
        for (ParchisBros::Iterator it = hijos.begin(); it != hijos.end(); ++it)
        {
            double aux = Poda_AlfaBeta(*it, jugador, profundidad + 1, profundidad_max, c_piece, id_piece, dice, alpha, beta, heuristic);

            if (aux > salida)
            {
                salida = aux;
                if (profundidad == 0)
                {
                    c_piece = it.getMovedColor();
                    id_piece = it.getMovedPieceId();
                    dice = it.getMovedDiceValue();
                }
            }

            alpha = max(alpha, salida);
            if (alpha >= beta)
                break;
        }

    else
    {
        salida = masinf;
        for (ParchisBros::Iterator it = hijos.begin(); it != hijos.end(); ++it)
        {
            double aux = Poda_AlfaBeta(*it, jugador, profundidad + 1, profundidad_max, c_piece, id_piece, dice, alpha, beta, heuristic);
            salida = min(aux, salida);
            beta = min(beta, salida);
            if (alpha >= beta)
                break;
        }
    }
    return salida;
}

void AIPlayer::think(color &c_piece, int &id_piece, int &dice) const
{
    switch (id)
    {
    case 0:
        thinkAlfaBeta(c_piece, id_piece, dice, ValoracionTest);
        break;
    case 1:
        thinkAlfaBeta(c_piece, id_piece, dice, Heuristica); // Esta es la mejor
        break;
    case 2:
        thinkFichaMasAdelantada(c_piece, id_piece, dice);
        break;
    case 3:
        thinkMejorOpcion(c_piece, id_piece, dice);
        break;
    }
}

double AIPlayer::ValoracionTest(const Parchis &estado, int jugador)
{
    // Heurística de prueba proporcionada para validar el funcionamiento del algoritmo de búsqueda.

    int ganador = estado.getWinner();
    int oponente = (jugador + 1) % 2;

    // Si hay un ganador, devuelvo más/menos infinito, según si he ganado yo o el oponente.
    if (ganador == jugador)
        return gana;
    else if (ganador == oponente)
        return pierde;
    else
    {
        // Colores que juega mi jugador y colores del oponente
        vector<color> my_colors = estado.getPlayerColors(jugador);
        vector<color> op_colors = estado.getPlayerColors(oponente);

        // Recorro todas las fichas de mi jugador
        int puntuacion_jugador = 0;
        // Recorro colores de mi jugador.
        for (int i = 0; i < my_colors.size(); i++)
        {
            color c = my_colors[i];
            // Recorro las fichas de ese color.
            for (int j = 0; j < num_pieces; j++)
            {
                // Valoro positivamente que la ficha esté en casilla segura o meta.
                if (estado.isSafePiece(c, j))
                    puntuacion_jugador++;
                else if (estado.getBoard().getPiece(c, j).get_box().type == goal)
                    puntuacion_jugador += 5;
            }
        }

        // Recorro todas las fichas del oponente
        int puntuacion_oponente = 0;
        // Recorro colores del oponente.
        for (int i = 0; i < op_colors.size(); i++)
        {
            color c = op_colors[i];
            // Recorro las fichas de ese color.
            for (int j = 0; j < num_pieces; j++)
                if (estado.isSafePiece(c, j))
                    // Valoro negativamente que la ficha esté en casilla segura o meta.
                    puntuacion_oponente++;
                else if (estado.getBoard().getPiece(c, j).get_box().type == goal)
                    puntuacion_oponente += 5;
        }

        // Devuelvo la puntuación de mi jugador menos la puntuación del oponente.
        return puntuacion_jugador - puntuacion_oponente;
    }
}

double AIPlayer::Heuristica(const Parchis &estado, int jugador)
{
    // Heurística de prueba proporcionada para validar el funcionamiento del algoritmo de búsqueda.

    const int ganador = estado.getWinner();
    const int oponente = (jugador + 1) % 2;

    // Si hay un ganador, devuelvo más/menos infinito, según si he ganado yo o el oponente.
    if (ganador == jugador)
        return gana;
    else if (ganador == oponente)
        return pierde;
    else
    {
        double puntuacion_jugador = PuntuacionJugador(estado, jugador, true);
        double puntuacion_oponente = PuntuacionJugador(estado, oponente);

        return puntuacion_jugador - puntuacion_oponente;
    }
}

double AIPlayer::PuntuacionJugador(const Parchis &estado, const int &jugador, const bool &esJugador)
{
    const double COLOR_GANADOR = 1.8, COLOR_PERDEDOR = 0.5;
    const double PODER = estado.getPower(jugador);
    const color color_actual = estado.getCurrentColor();
    double puntuacion = 0, puntuacion_colores[estado.getPlayerColors(jugador).size()] = {100};
    double bonif_comer = 15, bonif_destruir = 35, bonif_casilla = 20;
    if ((esJugador && jugador == 1) || (!esJugador && jugador == 0))
    {
        bonif_comer = 35;
        bonif_destruir = 15;
        bonif_casilla = 50;
        puntuacion += PODER;
    }

    // Comerse fichas
    double comer = 0;
    const color ultimo_color = get<0>(estado.getLastAction());
    switch (ultimo_color)
    {
    case red:
    case yellow:
        if (estado.eatenPiece().first == blue || estado.eatenPiece().first == green)
            comer += bonif_comer;
        else if (estado.eatenPiece().first != none)
            comer -= bonif_comer;
        break;
    case green:
    case blue:
        if (estado.eatenPiece().first == red || estado.eatenPiece().first == yellow)
            comer += bonif_comer;
        else if (estado.eatenPiece().first != none)
            comer -= bonif_comer;
        break;
    }
    double destruidas = 0;
    for (const pair<color, int> &pieza : estado.piecesDestroyedLastMove())
    {
        switch (pieza.first)
        {
        case red:
        case yellow:
            if (color_actual == blue || color_actual == green)
                destruidas += bonif_destruir;
            else
                destruidas -= bonif_destruir;
            break;
        case green:
        case blue:
            if (color_actual == red || color_actual == yellow)
                destruidas += bonif_destruir;
            else
                destruidas -= bonif_destruir;
            break;
        }
    }
    for (int i = 0; i < estado.getPlayerColors(jugador).size(); ++i)
    {
        const color c = estado.getPlayerColors(jugador)[i];
        for (int j = 0; j < num_pieces; ++j)
        {
            // Primer valor distancia
            puntuacion_colores[i] -= estado.distanceToGoal(c, j);

            const box_type tipo_casilla = estado.getBoard().getPiece(c, j).get_box().type;

            // Segundo valor si esta en meta
            if (tipo_casilla == goal)
                puntuacion_colores[i] += bonif_casilla;
            // Tercer valor si esta en casa
            else if (tipo_casilla == home)
                puntuacion_colores[i] -= bonif_casilla;
        }
    }

    if (puntuacion_colores[0] > puntuacion_colores[1])
        puntuacion += COLOR_GANADOR * puntuacion_colores[0] + COLOR_PERDEDOR * puntuacion_colores[1];
    else
        puntuacion += COLOR_GANADOR * puntuacion_colores[1] + COLOR_PERDEDOR * puntuacion_colores[0];

    if (PODER < 50)
        puntuacion += 10;
    else if (PODER < 60)
        puntuacion += 25;
    else if (PODER < 65)
        puntuacion -= 50;
    else if (PODER < 70)
        puntuacion += 40;
    else if (PODER < 75)
        puntuacion += 25;
    else if (PODER < 80)
        puntuacion += 60;
    else if (PODER < 85)
        puntuacion -= 60;
    else if (PODER < 90)
    {
        puntuacion += 60;
        if ((color_actual == yellow || color_actual == red) && (estado.piecesAtGoal(green) == 2 || estado.piecesAtGoal(blue) == 2))
            puntuacion += 30;
        else if ((color_actual == green || color_actual == blue) && (estado.piecesAtGoal(yellow) == 2 || estado.piecesAtGoal(red) == 2))
            puntuacion += 30;
    }
    else if (PODER < 95)
        puntuacion -= 70;
    else if (PODER < 100)
        puntuacion += 80;
    else
        puntuacion -= 80;

    return (puntuacion + ((comer + destruidas) * (COLOR_GANADOR + COLOR_PERDEDOR)));
}