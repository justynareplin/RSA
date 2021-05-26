#include <iostream>
#include <string>
#include < cstdlib>
using namespace std;

#include "olcConsoleGameEngine.h"

struct sKulka
{
	float px, py;
	float vx, vy;
	float ax, ay;
	float promien;
	float mass;

	int id;
};

class SilnikFizyczny : public olcConsoleGameEngine
{
public:
	SilnikFizyczny()
	{
		m_sAppName = L"Silnik fizyczny; symulacja zderzen";
	}

private:
	vector<pair<float, float>> kulkaModel;
	vector<sKulka> vectorKulek;
	sKulka *pWybranaKulka = nullptr;

	// Adding a ball
	void DodajKulke(float x, float y, float r = 8.0f)
	{
		sKulka b;
		b.px = x; b.py = y;
		b.vx = 0; b.vy = 0;
		b.ax = 0; b.ay = 0;
		b.promien = r;
		b.mass = r * 10.0f;

		b.id = vectorKulek.size();
		vectorKulek.emplace_back(b);
	}


public:
	bool OnUserCreate()
	{
		//tworzenie kulek
		kulkaModel.push_back({ 10.0f, 0.0f });
		int nPoints = 20;
		for (int i = 0; i < nPoints; i++)
			kulkaModel.push_back({ cosf(i / (float)(nPoints - 1) * 2.0f * 3.14159f) , sinf(i / (float)(nPoints - 1) * 2.0f * 3.14159f) });

		float fDefaultRad = 10.0f;
	
		// tworzenie kulek
		for (int i = 0; i < 6; i++)
			DodajKulke(rand() % ScreenWidth(), rand() % ScreenHeight(), rand() % 16 + 2);

		return true;
	}

	bool OnUserUpdate(float fElapsedTime)
	{
		auto czyZderzenie= [](float x1, float y1, float r1, float x2, float y2, float r2)
		{
			return fabs((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2)) <= (r1 + r2)*(r1 + r2);
		};

		auto klikMyszki = [](float x1, float y1, float r1, float px, float py)
		{
			return fabs((x1 - px)*(x1 - px) + (y1 - py)*(y1 - py)) < (r1 * r1);
		};

		if (m_mouse[0].bPressed || m_mouse[1].bPressed)
		{
			pWybranaKulka = nullptr;
			for (auto &kulka : vectorKulek)
			{
				if (klikMyszki(kulka.px, kulka.py, kulka.promien, m_mousePosX, m_mousePosY))
				{
					pWybranaKulka = &kulka;
					break;
				}
			}
		}

		if (m_mouse[0].bHeld)
		{
			if (pWybranaKulka != nullptr)
			{
				pWybranaKulka->px = m_mousePosX;
				pWybranaKulka->py = m_mousePosY;
			}
		}

		if (m_mouse[0].bReleased)
		{
			pWybranaKulka = nullptr;
		}

		if (m_mouse[1].bReleased)
		{
			if (pWybranaKulka != nullptr)
			{
				//  predkosc
				pWybranaKulka->vx = 5.0f * ((pWybranaKulka->px) - (float)m_mousePosX);
				pWybranaKulka->vy = 5.0f * ((pWybranaKulka->py) - (float)m_mousePosY);
			}

			pWybranaKulka = nullptr;
		}


		vector<pair<sKulka*, sKulka*>> vectorKolidujaceKulki;

		// update ekranu
		for (auto &kulka : vectorKulek)
		{
			// zmniejszenie przyspieszenia
			kulka.ax = -kulka.vx * 0.8f;
			kulka.ay = -kulka.vy * 0.8f;

			// predkosc kulek
			kulka.vx += kulka.ax * fElapsedTime;
			kulka.vy += kulka.ay * fElapsedTime;
			kulka.px += kulka.vx * fElapsedTime;
			kulka.py += kulka.vy * fElapsedTime;

			// jesli poza ekran
			if (kulka.px < 0) kulka.px += (float)ScreenWidth();
			if (kulka.px >= ScreenWidth()) kulka.px -= (float)ScreenWidth();
			if (kulka.py < 0) kulka.py += (float)ScreenHeight();
			if (kulka.py >= ScreenHeight()) kulka.py -= (float)ScreenHeight();

			//  jesli wektor predkosci bliski zeru 
			if (fabs(kulka.vx*kulka.vx + kulka.vy*kulka.vy) < 0.01f)
			{
				kulka.vx = 0;
				kulka.vy = 0;
			}
		}

		// kolizja static ,zderzenie
		for (auto &kulka : vectorKulek)
		{
			for (auto &target : vectorKulek)
			{
				if (kulka.id != target.id)
				{
					if (czyZderzenie(kulka.px, kulka.py, kulka.promien, target.px, target.py, target.promien))
					{
						// kolizja
						vectorKolidujaceKulki.push_back({ &kulka, &target });

						// dystans - odleglosc miedzy punktow centrsalnych kulek
						float fDistance = sqrtf((kulka.px - target.px)*(kulka.px - target.px) + (kulka.py - target.py)*(kulka.py - target.py));

						// zderzenie
						float fZderzenie = 0.5f * (fDistance - kulka.promien - target.promien);

						// po kolizji - 
						kulka.px -= fZderzenie * (kulka.px - target.px) / fDistance;
						kulka.py -= fZderzenie * (kulka.py - target.py) / fDistance;

						// po kolizji 
						target.px += fZderzenie * (kulka.px - target.px) / fDistance;
						target.py += fZderzenie * (kulka.py - target.py) / fDistance;
					}
				}
			}
		}

		// kolizja dynamiczna  
		for (auto c : vectorKolidujaceKulki)
		{
			sKulka *b1 = c.first;
			sKulka *b2 = c.second;

			//odleglosc miedzy kulkami 
			float fDistance = sqrtf((b1->px - b2->px)*(b1->px - b2->px) + (b1->py - b2->py)*(b1->py - b2->py));

			// Normal
			float nx = (b2->px - b1->px) / fDistance;
			float ny = (b2->py - b1->py) / fDistance;

			// odbicie pod katem prostym
			float tx = -ny;
			float ty = nx;
	
		}

		// Clear Screen
		Fill(0, 0, ScreenWidth(), ScreenHeight(), ' ');

		// rysowanie kulek
		for (auto kulka : vectorKulek)
			DrawWireFrameModel(kulkaModel, kulka.px, kulka.py, atan2f(kulka.vy, kulka.vx), kulka.promien, FG_WHITE);

		// Draw static collisions
		for (auto c : vectorKolidujaceKulki)
			DrawLine(c.first->px, c.first->py, c.second->px, c.second->py, PIXEL_SOLID, FG_RED);

		// rysowanie blue
		if (pWybranaKulka != nullptr)
			DrawLine(pWybranaKulka->px, pWybranaKulka->py, m_mousePosX, m_mousePosY, PIXEL_SOLID, FG_BLUE);

		return true;
	}

};

int main()
{
	 SilnikFizyczny game;
	game.ConstructConsole(100, 80, 8, 8);
	game.Start();
	
	cin.get();

	return 0;
};

