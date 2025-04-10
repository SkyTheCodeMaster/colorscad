#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <limits>
#include <stdexcept>

#include <lib3mf_implicit.hpp>


float linearToSRGB(float linear)
{
  if (linear <= 0.0031308) {
    return linear * 12.92f;
  }
  const float a = 0.055f;
  return static_cast<float>((1.0 + a) * std::pow(linear, 1/2.4) - a);
}

// Rotate indices (reorder) such that the smallest one is at index 0, preserving relative order
void rotate_indices(Lib3MF::sTriangle& triangle)
{
  auto& idx = triangle.m_Indices;
  if ((idx[1] < idx[0]) && (idx[1] < idx[2])) {
    Lib3MF_uint32 t = idx[0];
    idx[0] = idx[1];
    idx[1] = idx[2];
    idx[2] = t;
  } else if ((idx[2] < idx[0]) && (idx[2] < idx[1])) {
    Lib3MF_uint32 t = idx[2];
    idx[2] = idx[1];
    idx[1] = idx[0];
    idx[0] = t;
  }
}

std::string replace_all(std::string s, const std::string& key, const std::string& replacement)
{
  size_t pos = s.find(key);
  while (pos != std::string::npos) {
    s.replace(pos, key.size(), replacement);
    pos = s.find(key, pos + replacement.size());
  }
  return s;
}

// Convert RGB values to names
std::unordered_map<std::string, std::string> colors = {
  {"aliceblue","f0f8ff"},
  {"antiquewhite","faebd7"},
  {"aqua","00ffff"},
  {"aquamarine","7fffd4"},
  {"azure","f0ffff"},
  {"beige","f5f5dc"},
  {"bisque","ffe4c4"},
  {"black","000000"},
  {"blanchedalmond","ffebcd"},
  {"blue","0000ff"},
  {"blueviolet","8a2be2"},
  {"brown","a52a2a"},
  {"burlywood","deb887"},
  {"cadetblue","5f9ea0"},
  {"chartreuse","7fff00"},
  {"chocolate","d2691e"},
  {"coral","ff7f50"},
  {"cornflowerblue","6495ed"},
  {"cornsilk","fff8dc"},
  {"crimson","dc143c"},
  {"cyan","00ffff"},
  {"darkblue","00008b"},
  {"darkcyan","008b8b"},
  {"darkgoldenrod","b8860b"},
  {"darkgray","a9a9a9"},
  {"darkgreen","006400"},
  {"darkgrey","a9a9a9"},
  {"darkkhaki","bdb76b"},
  {"darkmagenta","8b008b"},
  {"darkolivegreen","556b2f"},
  {"darkorange","ff8c00"},
  {"darkorchid","9932cc"},
  {"darkred","8b0000"},
  {"darksalmon","e9967a"},
  {"darkseagreen","8fbc8f"},
  {"darkslateblue","483d8b"},
  {"darkslategray","2f4f4f"},
  {"darkslategrey","2f4f4f"},
  {"darkturquoise","00ced1"},
  {"darkviolet","9400d3"},
  {"deeppink","ff1493"},
  {"deepskyblue","00bfff"},
  {"dimgray","696969"},
  {"dimgrey","696969"},
  {"dodgerblue","1e90ff"},
  {"firebrick","b22222"},
  {"floralwhite","fffaf0"},
  {"forestgreen","228b22"},
  {"fuchsia","ff00ff"},
  {"gainsboro","dcdcdc"},
  {"ghostwhite","f8f8ff"},
  {"gold","ffd700"},
  {"goldenrod","daa520"},
  {"gray","808080"},
  {"green","008000"},
  {"greenyellow","adff2f"},
  {"grey","808080"},
  {"honeydew","f0fff0"},
  {"hotpink","ff69b4"},
  {"indianred","cd5c5c"},
  {"indigo","4b0082"},
  {"ivory","fffff0"},
  {"khaki","f0e68c"},
  {"lavender","e6e6fa"},
  {"lavenderblush","fff0f5"},
  {"lawngreen","7cfc00"},
  {"lemonchiffon","fffacd"},
  {"lightblue","add8e6"},
  {"lightcoral","f08080"},
  {"lightcyan","e0ffff"},
  {"lightgoldenrodyellow","fafad2"},
  {"lightgray","d3d3d3"},
  {"lightgreen","90ee90"},
  {"lightgrey","d3d3d3"},
  {"lightpink","ffb6c1"},
  {"lightsalmon","ffa07a"},
  {"lightseagreen","20b2aa"},
  {"lightskyblue","87cefa"},
  {"lightslategray","778899"},
  {"lightslategrey","778899"},
  {"lightsteelblue","b0c4de"},
  {"lightyellow","ffffe0"},
  {"lime","00ff00"},
  {"limegreen","32cd32"},
  {"linen","faf0e6"},
  {"magenta","ff00ff"},
  {"maroon","800000"},
  {"mediumaquamarine","66cdaa"},
  {"mediumblue","0000cd"},
  {"mediumorchid","ba55d3"},
  {"mediumpurple","9370db"},
  {"mediumseagreen","3cb371"},
  {"mediumslateblue","7b68ee"},
  {"mediumspringgreen","00fa9a"},
  {"mediumturquoise","48d1cc"},
  {"mediumvioletred","c71585"},
  {"midnightblue","191970"},
  {"mintcream","f5fffa"},
  {"mistyrose","ffe4e1"},
  {"moccasin","ffe4b5"},
  {"navajowhite","ffdead"},
  {"navy","000080"},
  {"oldlace","fdf5e6"},
  {"olive","808000"},
  {"olivedrab","6b8e23"},
  {"orange","ffa500"},
  {"orangered","ff4500"},
  {"orchid","da70d6"},
  {"palegoldenrod","eee8aa"},
  {"palegreen","98fb98"},
  {"paleturquoise","afeeee"},
  {"palevioletred","db7093"},
  {"papayawhip","ffefd5"},
  {"peachpuff","ffdab9"},
  {"peru","cd853f"},
  {"pink","ffc0cb"},
  {"plum","dda0dd"},
  {"powderblue","b0e0e6"},
  {"purple","800080"},
  {"red","ff0000"},
  {"rosybrown","bc8f8f"},
  {"royalblue","4169e1"},
  {"saddlebrown","8b4513"},
  {"salmon","fa8072"},
  {"sandybrown","f4a460"},
  {"seagreen","2e8b57"},
  {"seashell","fff5ee"},
  {"sienna","a0522d"},
  {"silver","c0c0c0"},
  {"skyblue","87ceeb"},
  {"slateblue","6a5acd"},
  {"slategray","708090"},
  {"slategrey","708090"},
  {"snow","fffafa"},
  {"springgreen","00ff7f"},
  {"steelblue","4682b4"},
  {"tan","d2b48c"},
  {"teal","008080"},
  {"thistle","d8bfd8"},
  {"tomato","ff6347"},
  {"turquoise","40e0d0"},
  {"violet","ee82ee"},
  {"wheat","f5deb3"},
  {"white","ffffff"},
  {"whitesmoke","f5f5f5"},
  {"yellow","ffff00"},
  {"yellowgreen","9acd32"}
};

std::string rgbaToHex(const std::string& rgbaString) {
  std::istringstream iss(rgbaString);
  double r, g, b, a;

  char comma;
  if (!(iss >> r >> comma >> g >> comma >> b >> comma >> a)) {
    throw std::invalid_argument("Invalid RGBA string format");
  }

  r = std::max(0.0, std::min(r, 1.0));
  g = std::max(0.0, std::min(g, 1.0));
  b = std::max(0.0, std::min(b, 1.0));
  a = std::max(0.0, std::min(a, 1.0));

  unsigned char red   = static_cast<unsigned char>(r * 255);
  unsigned char green = static_cast<unsigned char>(g * 255);
  unsigned char blue  = static_cast<unsigned char>(b * 255);
  unsigned char alpha = static_cast<unsigned char>(a * 255);

  std::ostringstream oss;
  oss << 
    std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(red)
    << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(green)
    << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(blue);

  return oss.str();
}

void parseHexToRgb(const std::string& hex, unsigned char& r, unsigned char& g, unsigned char& b) {
  // Skip '#' if present
  size_t start = (hex[0] == '#') ? 1 : 0;
  
  // Convert hex substrings to integers
  r = static_cast<unsigned char>(std::stoi(hex.substr(start, 2), nullptr, 16));
  g = static_cast<unsigned char>(std::stoi(hex.substr(start + 2, 2), nullptr, 16));
  b = static_cast<unsigned char>(std::stoi(hex.substr(start + 4, 2), nullptr, 16));
}

double calculateColorDistance(const std::string& hex1, const std::string& hex2) {
  unsigned char r1, g1, b1;
  unsigned char r2, g2, b2;

  parseHexToRgb(hex1, r1, g1, b1);
  parseHexToRgb(hex2, r2, g2, b2);
  
  // Calculate Euclidean distance
  double dr = r1 - r2;
  double dg = g1 - g2;
  double db = b1 - b2;
  
  return sqrt(dr*dr + dg*dg + db*db);
}

std::string hexToName(const std::string& hexString) {
  auto closestPair = *std::min_element(
    colors.begin(),
    colors.end(),
    [&hexString](const std::pair<const std::string&, const std::string&> lhs,
      const std::pair<const std::string&, const std::string&> rhs) {
      return calculateColorDistance(hexString, lhs.second) <
        calculateColorDistance(hexString, rhs.second);
    }
  );

  return closestPair.first; // Return the color name
}

// Returns number of skipped input lines
int mergeModels(char* outputFile)
{
  int skipped = 0;
  Lib3MF::PWrapper wrapper = Lib3MF::CWrapper::loadLibrary();
  Lib3MF::PModel mergedModel = wrapper->CreateModel();
  Lib3MF::PComponentsObject mergedComponentsObject = mergedModel->AddComponentsObject();
  std::map<Lib3MF_uint32, std::string> id_to_name;  // Stores name for each component ID in the merged model
  for (std::string line; std::getline(std::cin, line);) {
    // Define new color, extracted from filename such as "[0, 0.25, 1, 1].3mf"
    Lib3MF_uint32 colorGroupID = -1;
    size_t col_start = line.find("[");
    size_t col_end = line.find("]");
    std::string component_name;
    if ((col_start == std::string::npos) || (col_end == std::string::npos)) {
      std::cerr << "Not coloring '" << line << "': filename doesn't contain proper square brackets" << std::endl;
    } else {
      std::string col = line.substr(col_start + 1, col_end - (col_start + 1));

      // Determine the component's name.
      // This name needs to be stored in multiple places to be picked up by the majority of programs.
      // TODO also allow setting a custom name directly from OpenSCAD code
      std::string color_hex = rgbaToHex(col);
      std::string color_name = hexToName(color_hex);
      component_name = "[" + color_name + "]";

      std::vector<float> cols;
      size_t prev = 0;
      while (true) {
        size_t pos = col.find(",", prev);
        std::string val = col.substr(prev, pos - prev);
        cols.push_back(std::stof(val));
        if (pos == std::string::npos) break;
        prev = pos + 1;
      }
      if (cols.size() != 4) {
        std::cerr << "Not coloring '" << line << "': filename doesn't mention exactly 4 RGBA values" << std::endl;
      } else {
        Lib3MF_single r = linearToSRGB(cols[0]);
        Lib3MF_single g = linearToSRGB(cols[1]);
        Lib3MF_single b = linearToSRGB(cols[2]);
        Lib3MF_single a = cols[3];
        Lib3MF::PColorGroup colorGroup = mergedModel->AddColorGroup();
        Lib3MF::sColor color = wrapper->FloatRGBAToColor(r, g, b, a);
        colorGroup->AddColor(color);
        colorGroupID = colorGroup->GetResourceID();
      }
    }

    try {
      // Load model
      Lib3MF::PModel model = wrapper->CreateModel();
      Lib3MF::PReader reader = model->QueryReader("3mf");
      reader->ReadFromFile(line);

      // Loop over its objects
      Lib3MF::PObjectIterator objectIterator = model->GetObjects();
      while (objectIterator->MoveNext()) {
        const Lib3MF::PObject& object = objectIterator->GetCurrentObject();
        if (object->IsMeshObject()) {
          Lib3MF::PMeshObject mesh = model->GetMeshObjectByID(object->GetResourceID());

          // Get the mesh
          std::vector<Lib3MF::sPosition> vertices;
          std::vector<Lib3MF::sTriangle> triangle_indices;
          mesh->GetVertices(vertices);
          mesh->GetTriangleIndices(triangle_indices);

          // Rotate triangle indices, and sort the triangle list
          // This is to have consistent output, useful for testing purposes
          for (auto& triangle : triangle_indices) {
            rotate_indices(triangle);
          }
          std::sort(triangle_indices.begin(), triangle_indices.end(),
            [](Lib3MF::sTriangle a, Lib3MF::sTriangle b) {
              auto& ai = a.m_Indices;
              auto& bi = b.m_Indices;
              if (ai[0] != bi[0]) {
                return ai[0] < bi[0];
              }
              if (ai[1] != bi[1]) {
                return ai[1] < bi[1];
              }
              return ai[2] < bi[2];
            }
          );

          // Add the mesh
          Lib3MF::PMeshObject newMesh = mergedModel->AddMeshObject();
          newMesh->SetGeometry(vertices, triangle_indices);

          // Set color
          if (colorGroupID != -1) { // If we managed to extract a color from the filename
            newMesh->SetObjectLevelProperty(colorGroupID, 1);
          }

          // This component name assignment works for Cura, PrusaSlicer, SuperSlicer
          newMesh->SetName(component_name);

          // Add to merged model
          Lib3MF::PComponent component = mergedComponentsObject->AddComponent(newMesh.get(), wrapper->GetIdentityTransform());

          // Store component's ID->name mapping for later
          Lib3MF_uint32 id = component->GetObjectResourceID();
          id_to_name.emplace(id, component_name);
        } else if (object->IsComponentsObject()) {
          std::cout << line << ": skipping component object #" << object->GetResourceID() << std::endl;
        } else {
          std::cout << line << ": skipping unknown object #" << object->GetResourceID() << std::endl;
        }
      }
    } catch (Lib3MF::ELib3MFException &e) {
      std::cerr << "Trouble while processing '" << line << "': " << e.what() << std::endl;
      std::cerr << "Will skip this file/color, and proceed anyway." << std::endl;
      skipped++;
    }
  }
  Lib3MF::PBuildItem buildItem = mergedModel->AddBuildItem(mergedComponentsObject.get(), wrapper->GetIdentityTransform());

  // Add metadata attachment defining the component names; this works for Bambu Studio, OrcaSlicer
  Lib3MF::PAttachment attachment = mergedModel->AddAttachment("Metadata/model_settings.config", "");
  std::stringstream model_settings_stream;
  model_settings_stream
      << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl
      << "<config>" << std::endl
      << "  <object id=\"" << buildItem->GetObjectResourceID() << "\">" << std::endl;
  for (const auto& pair : id_to_name) {
    std::string component_name = replace_all(pair.second, "&", "&amp;");
    component_name = replace_all(std::move(component_name), "\"", "&quot;");
    model_settings_stream
        << "    <part id=\"" << pair.first << "\" subtype=\"normal_part\">" << std::endl
        << "      <metadata key=\"name\" value=\"" << component_name << "\"/>" << std::endl
        << "    </part>" << std::endl;
  }
  model_settings_stream
      << "  </object>" << std::endl
      << "</config>" << std::endl;
  std::string model_settings = std::move(model_settings_stream.str());
  attachment->ReadFromBuffer(Lib3MF::CInputVector<Lib3MF_uint8>(
      reinterpret_cast<const Lib3MF_uint8*>(model_settings.data()), model_settings.size()
  ));

  Lib3MF::PWriter writer = mergedModel->QueryWriter("3mf");
  writer->WriteToFile(outputFile);
  return skipped;
}

int main(int argc, char** argv)
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " OUTPUT_FILE" << std::endl
              << "A list of filenames is read from stdin; these must be .3mf files." << std::endl
              << "After loading each file, its mesh gets assigned a color based on the filename; and finally, all the" << std::endl
              << "meshes are merged into one model, which is saved as OUTPUT_FILE." << std::endl
              << "OUTPUT_FILE must not yet exist." << std::endl
              << "Example input line (filename): '[1, 0, 0.5, 0.9].3mf'." << std::endl
              << "This would result in a color assignment of r=1, g=0, b=0.5, alpha=0.9." << std::endl;
    return 1;
  }
  try {
    int skipped = mergeModels(argv[1]);
    if (skipped > 0) {
      std::cerr << "Warning: " << skipped << " input files were skipped!" << std::endl;
      return 1;
    }
  } catch (Lib3MF::ELib3MFException &e) {
    std::cerr << e.what() << std::endl;
    return e.getErrorCode();
  }
  return 0;
}
