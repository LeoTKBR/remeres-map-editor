#include "main.h"
#include "map.h"
#include "export_map_image.h"
#include "gui.h"
#include "editor.h"
#include "editor_tabs.h"
#include "map_tab.h"
#include "map_drawer.h"
#include "map_display.h"

void ExportRenderedMapImage(Map* map, int z, const std::string& outputPath, wxAuiNotebookEvent& event) {
    wxLogMessage("Iniciando exportação da imagem do mapa em uma posição específica...");

    // Obter o editor atual a partir do evento
    EditorTab* editor_tab = g_gui.tabbook->GetTab(event.GetInt());
    if (!editor_tab) {
        wxLogError("EditorTab não encontrado.");
        return;
    }
    MapTab* map_tab = dynamic_cast<MapTab*>(editor_tab);
    if (!map_tab || !map_tab->GetMap()) {
        wxLogError("MapTab inválido ou mapa não encontrado.");
        return;
    }

    Editor* editor = map_tab->GetEditor();
    if (!editor) {
        wxLogError("Editor não encontrado.");
        return;
    }

    // Obter o canvas de mapa
    MapCanvas* canvas = map_tab->GetCanvas();
    if (!canvas) {
        wxLogError("Canvas não encontrado.");
        return;
    }

    // Obter a janela de mapa
    MapWindow* map_window = dynamic_cast<MapWindow*>(map_tab->GetView());
    if (!map_window) {
        wxLogError("MapWindow não encontrado.");
        return;
    }

    // Configurar o contexto OpenGL
    wxLogMessage("Configurando contexto OpenGL...");
    canvas->SetCurrent(*g_gui.GetGLContext(canvas));

    // Obter as dimensões do mapa
    const int tileSize = 32;  // Tamanho de cada tile em pixels
    const int sqmBlockSize = 10;  // Bloco de 10x10 SQMs
    const int blockPixelSize = sqmBlockSize * tileSize;  // 320x320 pixels por bloco
    const int widthInTiles = map->getWidth();
    const int heightInTiles = map->getHeight();
    const int width = widthInTiles * tileSize;
    const int height = heightInTiles * tileSize;

    wxLogMessage("Dimensões do mapa: %d x %d pixels.", width, height);

    // Dividir o mapa em blocos de 10x10 SQMs
    const int blocksX = (widthInTiles + sqmBlockSize - 1) / sqmBlockSize;
    const int blocksY = (heightInTiles + sqmBlockSize - 1) / sqmBlockSize;

    wxLogMessage("Dividindo o mapa em %d blocos horizontais e %d blocos verticais.", blocksX, blocksY);

    wxImage finalImage(width, height);
    unsigned char* finalBuffer = finalImage.GetData();

    // Usar o diretório de saída como base para os arquivos
    wxFileName outputDir(wxString::FromUTF8(outputPath));
    wxString outputFolder = outputDir.GetPath();
    wxLogMessage("Usando diretório de saída: %s", outputFolder);

    try {
        // Definir a posição específica para capturar a imagem
        const int targetX = 990;
        const int targetY = 1031;
        const int targetZ = 7;

        wxLogMessage("Capturando screenshot da posição [%d, %d, %d]...", targetX, targetY, targetZ);

        // Centralizar a câmera na posição especificada
        Position targetPosition(targetX, targetY, targetZ);
        map_window->SetScreenCenterPosition(targetPosition);

        // Caminho para o arquivo da imagem
        wxString imagePath = wxFileName(outputFolder, "screenshot_990_1031_7.png").GetFullPath();

        // Capturar a screenshot
        canvas->TakeScreenshot(imagePath, "PNG");

        // Verificar se o arquivo foi criado
        if (!wxFileExists(imagePath)) {
            wxLogError("Erro: O arquivo da imagem não foi criado: %s", imagePath);
            return;
        }

        wxLogMessage("Imagem da posição [%d, %d, %d] salva em: %s", targetX, targetY, targetZ, imagePath);

        wxLogMessage("Exportação da imagem concluída.");
    } catch (const std::exception& e) {
        wxLogError("Erro durante a exportação: %s", e.what());
    } catch (...) {
        wxLogError("Erro desconhecido durante a exportação da imagem.");
    }
}