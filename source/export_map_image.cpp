#include "main.h"
#include "map.h"
#include "export_map_image.h"
#include "gui.h"
#include "editor.h"
#include "editor_tabs.h"
#include "map_tab.h"
#include "map_drawer.h"
#include "map_display.h"

void ExportRenderedMapImage(Map* map, int z, const std::string& outputPath, wxAuiNotebookEvent& event, 
    int startX, int startY, int endX, int endY) {
wxLogMessage("Iniciando exportação de imagens do mapa em uma área predefinida...");

EditorTab* editor_tab = g_gui.tabbook->GetTab(event.GetInt());
if (!editor_tab) return;
MapTab* map_tab = dynamic_cast<MapTab*>(editor_tab);
if (!map_tab || !map_tab->GetMap()) return;
Editor* editor = map_tab->GetEditor();
if (!editor) return;
MapCanvas* canvas = map_tab->GetCanvas();
if (!canvas) return;
MapWindow* map_window = dynamic_cast<MapWindow*>(map_tab->GetView());
if (!map_window) return;

canvas->SetCurrent(*g_gui.GetGLContext(canvas));
g_gui.SetCurrentZoom(16.69);
const int blockWidthSQM = 584;
const int blockHeightSQM = 304;
const int imageWidth = 512;
const int imageHeight = 512;

startX = std::max(0, startX);
startY = std::max(0, startY);
endX = std::min(map->getWidth(), endX);
endY = std::min(map->getHeight(), endY);

int numBlocksX = ((endX - startX) + blockWidthSQM - 1) / blockWidthSQM;
int numBlocksY = ((endY - startY) + blockHeightSQM - 1) / blockHeightSQM;

// Imagem final
wxImage finalImage(imageWidth * numBlocksX, imageHeight * numBlocksY);
finalImage.SetRGB(wxRect(0, 0, finalImage.GetWidth(), finalImage.GetHeight()), 0, 0, 0);

int blockX = 0, blockY = 0;

for (int y = startY; y < endY; y += blockHeightSQM, ++blockY) {
blockX = 0;
for (int x = startX; x < endX; x += blockWidthSQM, ++blockX) {
int centerX = x + blockWidthSQM / 2;
int centerY = y + blockHeightSQM / 2;

wxLogMessage("Capturando imagem no bloco central [%d, %d, %d]...", centerX, centerY, z);
Position blockCenter(centerX, centerY, z);
map_window->SetScreenCenterPosition(blockCenter);
wxTheApp->Yield(true);
wxMilliSleep(1000);

wxFileName tempFile(wxString::FromUTF8(outputPath));
wxString tempPath = tempFile.GetFullPath();

canvas->TakeSateliteshot(tempPath, "PNG");

wxDir dir(tempPath);
wxString tempImageName;
bool found = dir.GetFirst(&tempImageName, "screenshot_*.png", wxDIR_FILES);
if (!found) {
wxLogError("Nenhuma screenshot encontrada no bloco [%d, %d].", x, y);
continue;
}

wxImage capturedImage(wxFileName(tempPath, tempImageName).GetFullPath());
if (!capturedImage.IsOk()) {
wxLogError("Erro ao carregar a imagem [%s]", tempImageName);
continue;
}

// Verificar se é preta
unsigned char* data = capturedImage.GetData();
bool isAllBlack = true;
for (int i = 0; i < capturedImage.GetWidth() * capturedImage.GetHeight() * 3; i += 3) {
if (data[i] != 0 || data[i + 1] != 0 || data[i + 2] != 0) {
isAllBlack = false;
break;
}
}

if (isAllBlack) {
wxLogMessage("Imagem no bloco [%d, %d] é completamente preta. Ignorando...", x, y);
wxRemoveFile(wxFileName(tempPath, tempImageName).GetFullPath());
continue;
}

// Copiar pixels para imagem final
for (int py = 0; py < imageHeight; ++py) {
for (int px = 0; px < imageWidth; ++px) {
unsigned char r = capturedImage.GetRed(px, py);
unsigned char g = capturedImage.GetGreen(px, py);
unsigned char b = capturedImage.GetBlue(px, py);
finalImage.SetRGB(blockX * imageWidth + px, blockY * imageHeight + py, r, g, b);
}
}

wxRemoveFile(wxFileName(tempPath, tempImageName).GetFullPath());
wxMilliSleep(1000);
}
}

wxString finalPath = wxString::Format("%s/z%d.png", outputPath, z);
if (!finalImage.SaveFile(finalPath, wxBITMAP_TYPE_PNG)) {
wxLogError("Erro ao salvar a imagem final em: %s", finalPath);
} else {
wxLogMessage("Imagem final salva com sucesso: %s", finalPath);
}

wxLogMessage("Exportação de todas as imagens concluída.");
}