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
    wxLogMessage("Iniciando exportação da imagem do mapa...");

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

    try {
        for (int blockY = 0; blockY < blocksY; ++blockY) {
            for (int blockX = 0; blockX < blocksX; ++blockX) {
                wxLogMessage("Renderizando bloco [%d, %d]...", blockX, blockY);

                int blockWidth = std::min(sqmBlockSize, widthInTiles - blockX * sqmBlockSize) * tileSize;
                int blockHeight = std::min(sqmBlockSize, heightInTiles - blockY * sqmBlockSize) * tileSize;

                // Criar um buffer para capturar a renderização do bloco
                std::unique_ptr<unsigned char[]> buffer(new unsigned char[blockWidth * blockHeight * 3]);

                // Configurar o MapDrawer
                MapDrawer drawer(canvas);
                DrawingOptions& options = drawer.getOptions();
                options.SetIngame();

                // Ajustar a viewport para renderizar apenas o bloco atual
                glViewport(0, 0, blockWidth, blockHeight);
                map_window->Scroll(blockX * sqmBlockSize * tileSize, blockY * sqmBlockSize * tileSize, false);

                // Renderizar o bloco do mapa
                drawer.SetupVars();
                drawer.SetupGL();
                drawer.Draw();
                glReadPixels(0, 0, blockWidth, blockHeight, GL_RGB, GL_UNSIGNED_BYTE, buffer.get());
                drawer.Release();

                // Copiar o bloco renderizado para a imagem final
                for (int y = 0; y < blockHeight; ++y) {
                    memcpy(
                        finalBuffer + ((blockY * sqmBlockSize * tileSize + y) * width + blockX * sqmBlockSize * tileSize) * 3,
                        buffer.get() + y * blockWidth * 3,
                        blockWidth * 3
                    );
                }
            }
        }

        // Salvar a imagem final como arquivo PNG
        wxLogMessage("Salvando imagem final em: %s", outputPath);
        if (finalImage.SaveFile(outputPath, wxBITMAP_TYPE_PNG)) {
            wxLogMessage("Imagem exportada com sucesso: %s", outputPath);
        } else {
            wxLogError("Erro ao salvar a imagem: %s", outputPath);
        }
    } catch (const std::bad_alloc&) {
        wxLogError("Erro: Falha na alocação de memória durante a exportação da imagem.");
    } catch (...) {
        wxLogError("Erro desconhecido durante a exportação da imagem.");
    }
}