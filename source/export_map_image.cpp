#include "main.h"
#include "map.h"
#include "export_map_image.h"
#include "gui.h"
#include "editor.h"
#include "editor_tabs.h"
#include "map_tab.h"
#include "map_drawer.h"
#include "live_tab.h"
#include "map_display.h"

void ExportRenderedMapImage(Map* map, int z, const std::string& outputPath, wxAuiNotebookEvent& event) {
    // Obter o editor atual a partir do evento
    EditorTab* editor_tab = g_gui.tabbook->GetTab(event.GetInt());
    MapTab* map_tab = dynamic_cast<MapTab*>(editor_tab);  // Verifica se é um MapTab
    if (!map_tab || !map_tab->IsUniqueReference() || !map_tab->GetMap()) {
        return;  // Se não for um MapTab válido, retorna
    }

    Editor* editor = map_tab->GetEditor();  // Obtém o editor
    if (!editor) return;

    // Obter o canvas de mapa
    MapCanvas* canvas = map_tab->GetCanvas();  // Usa GetCanvas, que é o método correto
    if (!canvas) return;

    // Ajuste da configuração de renderização
    MapDrawer drawer(canvas);  // Cria o drawer com o canvas do mapa

    // Definindo zoom e scroll
    float zoom = 1.0f;  // Defina o zoom conforme necessário
    int scrollX = 0;    // Defina a posição do scroll X conforme necessário
    int scrollY = 0;    // Defina a posição do scroll Y conforme necessário

    // Aqui, vamos utilizar os métodos de MapWindow para ajustar o tamanho e o scroll
    MapWindow* map_window = dynamic_cast<MapWindow*>(map_tab->GetView());
    if (map_window) {
        // Obter o tamanho da janela
        int windowWidth, windowHeight;
        map_window->GetSize(&windowWidth, &windowHeight); // Obtém o tamanho da janela

        map_window->SetSize(windowWidth, windowHeight, true);  // Ajusta o tamanho para centralizar o mapa
        map_window->UpdateScrollbarsPublic(windowWidth, windowHeight);  // Atualiza o scroll
    }

    // Obter as dimensões do mapa
    const int tileSize = 32;  // Tamanho de cada tile em pixels
    const int widthInTiles = map->getWidth();
    const int heightInTiles = map->getHeight();
    const int width = widthInTiles * tileSize;  // Largura do mapa em pixels
    const int height = heightInTiles * tileSize;  // Altura do mapa em pixels

    // Criar o bitmap e memória de renderização
    wxBitmap bitmap(width, height, 32);
    wxMemoryDC memDC;
    memDC.SelectObject(bitmap);
    memDC.SetBrush(*wxWHITE_BRUSH);  // Troque o fundo para branco
    memDC.Clear();

    // Renderizar o mapa usando o MapDrawer
    drawer.Draw();  // Aqui o MapDrawer irá desenhar o mapa no canvas

    // Finalizando o bitmap e salvando a imagem
    memDC.SelectObject(wxNullBitmap);
    wxImage image = bitmap.ConvertToImage();
    image.SaveFile(outputPath, wxBITMAP_TYPE_PNG);
    std::cout << "Map Width: " << width << ", Map Height: " << height << std::endl;
    std::cout << "Zoom: " << zoom << std::endl;

}