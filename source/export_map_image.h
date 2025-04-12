#ifndef RME_MAP_IMAGE_EXPORTER_H
#define RME_MAP_IMAGE_EXPORTER_H

#include <string>

class Map; // Declaração da classe Map, que você deve incluir no arquivo .cpp

void ExportRenderedMapImage(Map* map, int z, const std::string& outputPath, wxAuiNotebookEvent& event);

#endif