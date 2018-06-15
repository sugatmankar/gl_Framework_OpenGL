#pragma once
#include<windows.h>

#include <FrameworkHeader.h>

#include<map>
#include<math.h>
// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
//FreeType
#include <ft2build.h>
#include <freetype\freetype.h>


using namespace vmath;


class FontRenderer : public NFramework::TFrameworkMain {
public:
	FontRenderer();
	~FontRenderer();
};

