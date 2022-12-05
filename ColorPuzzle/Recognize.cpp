/*
Recognize.cpp
*/

#include <Windows.h>
#include <vector>
#include <map>
#include <algorithm>

#include "Recognize.h"

#include "SolvePuzzle.h"

static RecognizePuzzle recognizePuzzle;


void RecognizePuzzle::Start()
{
	refPoints.Start();
	gridPoints.clear();
}


void RecognizePuzzle::DetectPuzzle(HWND hWnd, const Point2D& p)
{
	refPoints.Add(p);
	if (refPoints.GetStatus() == REFPOINTNR) {
		HDC hDC = GetDC(hWnd);
		const std::array < Point2D, REFPOINTNR>& points = refPoints.GetRefPoints();
		for (auto p : points) {
			DrawCross(hWnd, hDC, 0xFFFFFF, p);
		}
		CalculateGridPoints(hWnd, hDC);
		ReleaseDC(hWnd, hDC);
	}
}

int RecognizePuzzle::CalculateGridPoints(HWND hWnd, HDC hDC)
{
	const std::array < Point2D, REFPOINTNR>& points = refPoints.GetRefPoints();
	double gridGapY  = abs(points[0][1] - points[1][1]);
	double gridGapX  = abs(points[0][0] - points[2][0]);
	double tubeGapY  = abs(points[0][1] - points[3][1]);
	double tubeNGapX  = abs(points[0][0] - points[3][0]);
	double tubeMGapX = abs(points[4][0] - points[5][0]);
	double tubeOGapX = abs(points[5][0] - points[6][0]);

	double t = tubeGapY / gridGapY;
	double n = tubeNGapX / gridGapX;
	double m = tubeMGapX / gridGapX;
	double o = tubeOGapX / gridGapX;

	int t1 = (int)(t + .5);
	int n1 = (int)(n + .5);
	int m1 = (int)(m + .5);
	int o1 = (int)(o + .5);

	int tSize = t1 + 1;
	int tNr = n1 + m1 + o1 + 2;

	int gridX = tubeNGapX / n1;
	int gridY = tubeGapY / t1;

	{
		char buffer[500];
		sprintf_s(buffer, "CalculateGridPints - t:%f, n:%f, m:%f, o:%f", t, n, m, o);
		OutputDebugStringA(buffer);
		sprintf_s(buffer, " tSize:%d, tNr:%d\n", tSize, tNr);
		OutputDebugStringA(buffer);

	}

	Point2D p0 (points[0]);
	for (int i = 0; i <= n1 + m1 + 1; ++i) {
		if (i == n1 + 1)
			p0 = points[4];
		for (int j = 0; j <= t1; ++j) {
			Point2D p(p0);
			p[1] += gridY * j;
			gridPoints.push_back (p);
		}
		p0[0] += gridX;
	}

	for (auto p : gridPoints) {
		DrawCross(hWnd, hDC, 0x00FFFF, p);
	}
	DetectColorClass(hWnd, hDC, tSize, tNr);

	return gridPoints.size();
}

int32_t ColorDiff(COLORREF c1, COLORREF c2)
{
	int r = ((c1 & 0xFF) - (c2 & 0xFF));
	int g = ((c1 >> 8) & 0xFF) - (((c2 >> 8) & 0xFF));
	int b = ((c1 >> 16) & 0xFF) - (((c2 >> 16) & 0xFF));
	return (r * r + g * g + b * b);
}


int32_t RecognizePuzzle::DetectColorClass(HWND hWnd, HDC hDC, int tSize, int tNr)
{
	std::map<int, int > mapIndexClass;
	std::vector<std::pair<int, int >> vectorOfPairClassIndex;
	std::vector<COLORREF> vectorOfcolor;
	int classID = 1;
	for (int i = 0; i < gridPoints.size(); ++i) {
		if (mapIndexClass.find(i) == mapIndexClass.end ()) {
			std::vector<std::pair<int, int>> vectorOfPairDistIndex;
			const int offset = 0;
			Point2D p0 = gridPoints[i];
			COLORREF color0 = GetPixel(hDC, p0[0] + offset, p0[1] + offset);
			for (int j = 0; j < gridPoints.size(); ++j) {
				Point2D p1 = gridPoints[j];
				COLORREF color1 = GetPixel(hDC, p1[0] + offset, p1[1] + offset);
				vectorOfPairDistIndex.push_back(std::pair<int, int>(ColorDiff(color0, color1), j));
			}
			std::sort(vectorOfPairDistIndex.begin(), vectorOfPairDistIndex.end());
			int n = 0;
			for (auto item : vectorOfPairDistIndex) {
				{
					char buffer[500];
					sprintf_s(buffer, sizeof(buffer), "list: i:%d, j:%d, diff:%d \n", i, item.second, item.first);
					OutputDebugStringA(buffer);
				}

				mapIndexClass[item.second] = classID;
				vectorOfPairClassIndex.push_back(std::pair<int, int >(classID, item.second));

				n++;
				if (n >= tSize)
					break;
			}
			vectorOfcolor.push_back(color0);
			++classID;
		}

	}

	std::sort(vectorOfPairClassIndex.begin(), vectorOfPairClassIndex.end());
	for (auto item : vectorOfPairClassIndex) {
		wchar_t buffer[500];
		wsprintf(buffer, L"item c:%d, i:%d\n", item.first, item.second);
		OutputDebugStringW(buffer);
	}

	std::string puzzleStart;
	std::string tube;

	for (auto item : mapIndexClass) {
		wchar_t buffer[500];
		wsprintf(buffer, L"item i:%d, c:%d\n", item.first, item.second);
		OutputDebugStringW(buffer);
		wsprintf(buffer, L"%d/%d", item.first + 1, item.second);

		Graphics graphics(hDC);

		SolidBrush  brush(Color(255, 255, 127, 127));
		FontFamily  fontFamily(L"Times New Roman");
		Font        font(&fontFamily, 24, FontStyleRegular, UnitPixel);
		float x = gridPoints[item.first][0];
		float y = gridPoints[item.first][1];
		PointF      pointF(x, y);
		graphics.DrawString(buffer, -1, &font, pointF, &brush);
		tube.append(1, char (item.second - 1 + 'A'));
		if (tube.size() == tSize) {
			for (auto it2 = tube.crbegin(); it2 != tube.crend(); ++it2)
				puzzleStart.append(1, *it2);
			tube.clear();
		}
	}

	StaticSetPuzzle(tSize, tNr, puzzleStart, &vectorOfcolor);
	StaticShowPuzzle(hWnd, hDC);

	return gridPoints.size();
}


int32_t RecognizePuzzle::CalcDiffSide(HDC hDC, const Point2D& p, int addX, int addY, Point2D& pSide)
{
	const int32_t w = 50;

	int32_t di = 0;
	Point2D dip1 = p;
	int32_t x = p[0];
	int32_t y = p[1];
	COLORREF col1 = GetPixel(hDC, x, y);
	for (int i = 1; i < w; ++i) {
		COLORREF col2 = GetPixel(hDC, x + 1 * addX, y + 1 * addY);
		int32_t r1 = (col2 & 0xff) - (col1 & 0xff);
		if (abs(r1) > di) {
			dip1 = { x + 1 * addX,y + 1 * addY };
			di = abs(r1);
		}
		r1 = ((col2 >> 8) & 0xff) - ((col1 >> 8) & 0xff);
		if (abs(r1) > di) {
			dip1 = { x + 1 * addX,y + 1 * addY };
			di = abs(r1);
		}
		r1 = ((col2 >> 16) & 0xff) - ((col1 >> 16) & 0xff);
		if (abs(r1) > di) {
			dip1 = { x + 1 * addX,y + 1 * addY };
			di = abs(r1);
		}
		x = x + 1 * addX;
		y = y + 1 * addY;
		col1 = col2;
		if (di > 30)
			break;
	}
	pSide = dip1;

	return di;
}


void RecognizePuzzle::DrawCross(HWND hWnd, HDC hDC, const COLORREF& col, const Point2D& p)
{
	Graphics graphics(hDC);
	Pen pen(Color(255, col&(0xff), (col >> 8) & (0xff), (col >> 16) & (0xff)));
	Point2D dipL = p;
	int32_t diff_l = CalcDiffSide(hDC, p, -1, 0, dipL);
	Point2D dipR = p;
	int32_t diff_r = CalcDiffSide(hDC, p,  1, 0, dipR);

	Point2D dipU = p;
	int32_t diff_u = CalcDiffSide(hDC, p, 0, -1, dipU);
	Point2D dipD = p;
	int32_t diff_d = CalcDiffSide(hDC, p, 0,  1, dipD);

//	graphics.DrawLine(&pen, dipL[0], dipL[1], dipR[0], dipR[1]);
//	graphics.DrawLine(&pen, dipU[0], dipU[1], dipD[0], dipD[1]);

}


int32_t RecognizePuzzle::StartShowRowStatus(int32_t rowSize)
{
	rowStatus.Start (rowSize);
	return 0;
}

int32_t RecognizePuzzle::ShowRawStatus(HWND hWnd, HDC hDC, const Point2D& p)
{
	int32_t y = p[1];
	const int32_t yDraw = 600;
	const int32_t yDrawSize = 200;
	Rect rectDraw = {0, yDraw -2, rowStatus.RowSize (), yDrawSize +2};

	Graphics graphics(hDC);
	SolidBrush  brush(Color(255, 0, 0, 0));
	graphics.FillRectangle(&brush, rectDraw);

	Pen      pen(Color(255, 255, 0, 0));
	int32_t x0 = 20;
	COLORREF color1 = GetPixel(hDC, x0, y);
	int32_t diff1 = 0;
	const int32_t div = 1024;
	for (int32_t xPos = x0 + 1; xPos < rowStatus.RowSize(); ++xPos) {
		COLORREF color2 = GetPixel(hDC, xPos, y);
		int32_t diff2 = ColorDiff(color1, color2);
		if (xPos > x0 + 1) {
			int32_t y1 = min(diff1 / div, yDrawSize) + yDraw;
			int32_t y2 = min(diff2 / div, yDrawSize) + yDraw;
			graphics.DrawLine(&pen, xPos - 1, y1, xPos, y2);
		}
		diff1 = diff2;
		color1 = color2;

	}
	return 0;
}


void DetectWidthHeight(HDC hDC, uint32_t left, uint32_t top, uint32_t& w, uint32_t& h)
{
	const uint32_t xMax = 1000;
	const uint32_t yMax = 1000;
	uint32_t y = 1;
	for (uint32_t x = 1; x < xMax; ++x) {
		COLORREF col = GetPixel(hDC, x + left, y + top);
		if ((col & 0xFFFFFF) == 0xFFFFFF) {
			w = x - 1;
			break;
		}

	}
	uint32_t x = 1;
	for (uint32_t y = 1; y < yMax; ++y) {
		COLORREF col = GetPixel(hDC, x + left, y + top);
		if ((col & 0xFFFFFF) == 0xFFFFFF) {
			h = y - 1;
			break;
		}
	}
	char buffer[500];
	sprintf_s(buffer, "DetectWidthHeight - w:%d h:%d \r\n", w, h);
	OutputDebugStringA(buffer);
}


int RecognizePuzzle::GetDiff(HDC hDC, uint32_t y, std::vector<int32_t>& diffVector)
{
	uint32_t w = max (imageRect.GetRight(), imageRectCorrected.GetRight () - 15);
	uint32_t left = imageRect.GetLeft();
	uint32_t top = imageRect.GetTop();
	diffVector.clear();
	int32_t diff1 = 0;
	COLORREF color1;
	for (uint32_t x = 0; x < w; ++x) {
		COLORREF color2 = GetPixel(hDC, x + left, y + top);
		int32_t diff2 = 0;
		if (x > imageRect.GetLeft() + 1) {
			diff2 = ColorDiff (color1, color2);
			diffVector.push_back(diff2);
		}
		else {
			diffVector.push_back(0);
		}
		color1 = color2;
	}
	return diffVector.size();
}


int RecognizePuzzle::DetectTube(const std::vector<int32_t>& diffVector, uint32_t y, std::vector<int32_t>& tubeXpos, int32_t& tubeWidthPixel)
{
	if (y == 51) {
		char buffer[500];
		sprintf_s(buffer, "DetectTube - y:%d\r\n", y);
		OutputDebugStringA(buffer);
	}
	tubeWidthPixel = 0;
	FilterAdditive twpFilter;
	tubeXpos.clear();
	const int32_t limitDiff = 1024;
	const int32_t maxW = 10;
	const int32_t maxDiff = 8;
	const int32_t minGap = 30;
	const int32_t maxGap = 90;
	std::vector<int32_t> peaks;
	std::vector<int32_t> mainPeaks;
	for (uint32_t i = 0; i < diffVector.size(); ++i) {
		if (diffVector[i] > limitDiff)
			peaks.push_back(i);
	}
	uint32_t j = 0;
	std::vector<int32_t> minMaxPeaks;
	for (uint32_t i = 1; i < peaks.size(); ++i) {
		if ((peaks[i] - peaks[j]) < maxW) {
			if (minMaxPeaks.size() == 0 || minMaxPeaks[minMaxPeaks.size()-1] != j) {
				minMaxPeaks.push_back(j);
			}
			else {
//				minMaxPeaks.push_back(j);
			}
		}
		else
			j = i;
	}
	std::vector<int32_t> averPeaks;
	for (uint32_t i = 0; i < minMaxPeaks.size(); ++i) {
		uint32_t i1 = minMaxPeaks[i];
		uint32_t i2 = peaks.size() - 1;
		if ((i + 1) < minMaxPeaks.size ())
			i2 = minMaxPeaks[i + 1] - 1;
		averPeaks.push_back((i1 + i2) / 2);
	}
	int32_t tubeW = 0;
	int32_t tubeG = 0;
	int32_t tubeNr = 0;
	bool detected = false;
	if (averPeaks.size() >= 6 && (averPeaks.size() % 2) == 0) {
		for (uint32_t i = 0; i < averPeaks.size() - 1; i += 2) {
			if (i == 0) {
				tubeW = peaks[averPeaks[i + 1]] - peaks[averPeaks[i]];
				twpFilter.Add(tubeW);
				detected = true;
				tubeNr++;
			}
			else if (tubeG == 0) {
				tubeG = peaks[averPeaks[i]] - peaks[averPeaks[i - 1]];
			}
			if (i > 0) {
				int32_t tW = peaks[averPeaks[i + 1]] - peaks[averPeaks[i]];
				twpFilter.Add(tW);
				int32_t tG = peaks[averPeaks[i]] - peaks[averPeaks[i - 1]];
				if (abs(tW - tubeW) < maxDiff && abs(tG - tubeG) < maxDiff) {
					tubeNr++;
				}
				else {
					detected = false;
					break;
				}
			}
		}
	}
	if (detected) {
		tubeWidthPixel = twpFilter.Get();
		for (uint32_t i = 0; i < averPeaks.size (); i += 2) {
			tubeXpos.push_back((peaks[averPeaks[i + 1]] + peaks[averPeaks[i]]) / 2);
		}
		char buffer[500];
		sprintf_s(buffer, "detected - y:%d t:%d w:%d g:%d\r\n", y, tubeNr, tubeW, tubeG);
		OutputDebugStringA(buffer);
	}
	return detected;
}

bool RecognizePuzzle::IsEmpty(HDC hDC, const std::vector < Point2D>& points) 
{
	int32_t diffSum = 0;
	const int32_t diffMax = 10000;
	bool first = true;
	COLORREF c1 = 0;
	std::vector <COLORREF> colors; // for debug
	for (auto item : points) {
		COLORREF c2 = GetPixel(hDC, item[0], item[1]);
		if (!first)
			diffSum += ColorDiff(c1, c2);
		c1 = c2;
		first = false;

		colors.push_back(c1); // for debug
	}
	if (diffSum < diffMax) {
		char buffer[500];
		sprintf_s(buffer, "IsEmpty true:%d ", diffSum);
		OutputDebugStringA(buffer);
		for (auto c : colors) {
			sprintf_s(buffer, " %x.%x.%x,", c & 0xff, (c >> 8) & 0xff, (c >> 16) & 0xff);
			OutputDebugStringA(buffer);
		}
		OutputDebugStringA("\r\n");
	}
	else {
		char buffer[500];
		sprintf_s(buffer, "IsEmpty false:%d ", diffSum);
		OutputDebugStringA(buffer);
		for (auto c : colors) {
			sprintf_s(buffer, " %x.%x.%x,", c & 0xff, (c >> 8) & 0xff, (c >> 16) & 0xff);
			OutputDebugStringA(buffer);
		}
		OutputDebugStringA("\r\n");
	}
	return (diffSum < diffMax);
}

bool RecognizePuzzle::GeneratePuzzle(HWND hWnd, int32_t tubeWidth, const std::vector < std::pair<std::vector<int32_t>, int32_t>>& tubeXYpos)
{
	int32_t addX = imageRect.GetLeft();
	int32_t addY = imageRect.GetTop();
	std::vector<TubeGroup> groups;
	for (auto it : tubeXYpos) {
		if (groups.size() == 0) {
			TubeGroup gr;
			gr.top = it.second;
			gr.bottom = it.second;
			gr.xMainVector = it.first;
			groups.push_back(gr);
		}
		else {
			if (labs(groups.back().bottom - it.second) < tubeWidth &&
				groups.back().xMainVector.size() == it.first.size())
				groups.back().bottom = it.second;
			else {
				TubeGroup gr;
				gr.top = it.second;
				gr.bottom = it.second;
				gr.xMainVector = it.first;
				groups.push_back(gr);
			}
		}
	}
	FilterAdditive tubeSizePixel;
	for (auto it : groups) {
		tubeSizePixel.Add(it.bottom - it.top);
	}
	int32_t tubeSize = tubeSizePixel.Get() / tubeWidth;

	HDC hDC = GetDC(hWnd);
	Graphics graphics(hDC);
	Pen      pen(Color(255, 0, 0, 255));

	int32_t len = tubeSizePixel.Get() / tubeSize;
	int32_t len2 = len / 2;
	int32_t tubeNr = 0;
	for (auto it : groups) {
		tubeSizePixel.Add(it.bottom - it.top);
		tubeNr += it.xMainVector.size();
		for (int j = 0; j < it.xMainVector.size(); ++j) {
			int32_t x0 = it.xMainVector[j] + addX;
			std::vector < Point2D> gridPointsTmp;
			for (int i = 0; i < tubeSize; ++i) {
				int32_t y0 = it.top + addY + ((2 * i + 1) * len / 2);
				Point2D p = { x0, y0 + 10 };
				gridPointsTmp.push_back(p);
//				graphics.DrawLine(&pen, x0 - len2, y0 - len2, x0 + len2, y0 + len2);
//				graphics.DrawLine(&pen, x0 - len2, y0 + len2, x0 + len2, y0 - len2);
			}
			if (!IsEmpty(hDC, gridPointsTmp)) {
				for (auto item : gridPointsTmp)
					gridPoints.push_back(item);
			}
		}
	}
	DetectColorClass(hWnd, hDC, tubeSize, tubeNr);
	ReleaseDC(hWnd, hDC);
	return true;
}

int RecognizePuzzle::RecognizePicturePuzzle(HWND hWnd)
{
	if (imageRect.GetLeft() == 0)
		return -1;

	std::vector<int32_t> diffVector;
	std::vector < std::pair<std::vector<int32_t>, int32_t>> tubeXYpos;
	std::vector<int32_t> tubeXpos;
	std::vector<int32_t> tubeYposMin;
	std::vector<int32_t> tubeYpos;
	int32_t xSize = 0;
	int32_t ySize = 0;
	int32_t yMin = imageRect.GetLeft();
	int32_t yMax = imageRect.GetRight();

	const int32_t minLimit = 10;

	HDC hDC = GetDC(hWnd);

	uint32_t w = 0;
	uint32_t h = 0;

	FilterAdditive wFilter;
	DetectWidthHeight(hDC, imageRect.GetLeft(), imageRect.GetTop(), w, h);
	imageRectCorrected = Rect(imageRect.GetLeft(), imageRect.GetTop(), w, h);

	if (imageRectCorrected.GetRight () > imageRect.GetRight())
		yMax = imageRectCorrected.GetRight();
	for (uint32_t y = yMin; y < yMax - 1; ++y) {
		GetDiff(hDC, y, diffVector);
		int32_t tubeWidtPixel = 0;
		if (DetectTube(diffVector, y, tubeXpos, tubeWidtPixel) && tubeWidtPixel > 0) {
			if (tubeYposMin.size() == 0) {
				tubeYposMin.push_back(y);
				std::pair <std::vector<int32_t>, int32_t> xy{ tubeXpos , y};
				tubeXYpos.push_back(xy);
				wFilter.Add(tubeWidtPixel);
			}
			else if (labs(tubeYposMin[tubeYposMin.size() - 1] - y) > minLimit) {
				tubeYposMin.push_back(y);
				std::pair <std::vector<int32_t>, int32_t> xy{ tubeXpos , y };
				tubeXYpos.push_back(xy);
				wFilter.Add(tubeWidtPixel);
			}
		}
	}
	for (auto it1 : tubeXYpos) {
		char buffer[500];
		sprintf_s(buffer, "XY pos - y:%d -", it1.second);
		OutputDebugStringA(buffer);
		for (auto it2 : it1.first) {
			sprintf_s(buffer, ", %d", it2);
			OutputDebugStringA(buffer);
		}
		OutputDebugStringA("\r\n");
	}
	GeneratePuzzle(hWnd, wFilter.Get(), tubeXYpos);
	ReleaseDC(hWnd, hDC);
	return 0;
}


int InputReferencePointsMouse(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message != WM_RBUTTONDOWN)
		return -1;

	WPARAM xPos = LOWORD(lParam);
	WPARAM yPos = HIWORD(lParam);
	HDC hDC = GetDC(hWnd);
	COLORREF color = GetPixel(hDC, xPos, yPos);
	ReleaseDC(hWnd, hDC);
	Point2D p = { xPos , yPos };
	recognizePuzzle.DetectPuzzle(hWnd, p);
	return 0;
}


int InputReferencePointsKeyboard(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_CHAR && (wParam == '1')) {
		recognizePuzzle.Start();
		return 0;
	}
	return -1;
}

int ShowRawStatusMouse(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message != WM_RBUTTONDOWN)
		return -1;

	if (!recognizePuzzle.ShowRawState ())
		return -1;

	WPARAM xPos = LOWORD(lParam);
	WPARAM yPos = HIWORD(lParam);
	HDC hDC = GetDC(hWnd);
	COLORREF color = GetPixel(hDC, xPos, yPos);
	Point2D p = { xPos , yPos };
	recognizePuzzle.ShowRawStatus(hWnd, hDC, p);
	ReleaseDC(hWnd, hDC);
	return 0;
}


int ShowRowStatusKeyboard()
{
	recognizePuzzle.StartShowRowStatus(800);
	return 0;
}


void SetImageRect(Rect& imageRect)
{
	recognizePuzzle.SetImageRect(imageRect);
}

void RecognizePicturePuzzle(HWND hWnd)
{
	recognizePuzzle.RecognizePicturePuzzle(hWnd);
}