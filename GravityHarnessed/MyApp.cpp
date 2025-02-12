#include "MyApp.h"
#include "SDL_GLDebugMessageCallback.h"
#include "ObjParser.h"
#include "ParametricSurfaceMesh.hpp"
#include "ProgramBuilder.h"

#include <imgui.h>

#include <string>
#include <array>
#include <algorithm>

CMyApp::CMyApp()
{
}

CMyApp::~CMyApp()
{
}

void CMyApp::SetupDebugCallback()
{
	// engedélyezzük és állítsuk be a debug callback függvényt ha debug context-ben vagyunk 
	GLint context_flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &context_flags);
	if (context_flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
		glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, GL_DONT_CARE, 0, nullptr, GL_FALSE);
		glDebugMessageCallback(SDL_GLDebugMessageCallback, nullptr);
	}
}

void CMyApp::InitShaders()
{
	m_ProgramPlanetID = glCreateProgram();
	ProgramBuilder{ m_ProgramPlanetID }
		.ShaderStage( GL_VERTEX_SHADER, "Shaders/Vert_PlanetPosNormTex.vert" )
		.ShaderStage( GL_FRAGMENT_SHADER, "Shaders/Frag_Lighting.frag" )
		.Link();

	m_ProgramSunID = glCreateProgram();
	ProgramBuilder{ m_ProgramSunID }
		.ShaderStage(GL_VERTEX_SHADER, "Shaders/Vert_SunPosNormTex.vert")
		.ShaderStage(GL_FRAGMENT_SHADER, "Shaders/Frag_Sun.frag")
		.Link();

	m_ProgramSkyboxID = glCreateProgram();
	ProgramBuilder{ m_ProgramSkyboxID }
		.ShaderStage(GL_VERTEX_SHADER, "Shaders/SkyBox/Vert_skybox.vert")
		.ShaderStage(GL_FRAGMENT_SHADER, "Shaders/SkyBox/Frag_skybox.frag")
		.Link();
}

void CMyApp::CleanShaders()
{
	glDeleteProgram( m_ProgramPlanetID );
	glDeleteProgram(m_ProgramSkyboxID);
}


// Nyers parameterek
struct Param
{
	glm::vec3 GetPos( float u, float v ) const noexcept
	{
		glm::vec3 b[3][3] = { {glm::vec3(0,0,0),glm::vec3(1,0,0),glm::vec3(2,0,0)},
							  {glm::vec3(0,0,1),glm::vec3(1,2,1),glm::vec3(2,0,1)},
							  {glm::vec3(0,0,2),glm::vec3(1,0,2),glm::vec3(2,0,2)} };
		return glm::vec3( u, v, 0.0f );
	}
	glm::vec3 GetNorm( float u, float v ) const noexcept
	{
		return glm::vec3( 0.0,0.0,1.0 );
	}
	glm::vec2 GetTex( float u, float v ) const noexcept
	{
		return glm::vec2( u, v );
	}
};

void CMyApp::InitGeometry()
{

	const std::initializer_list<VertexAttributeDescriptor> vertexAttribList =
	{
		{ 0, offsetof( Vertex, position ), 3, GL_FLOAT },
		// { 1, offsetof( Vertex, normal   ), 3, GL_FLOAT },
		{ 1, offsetof( Vertex, texcoord ), 2, GL_FLOAT },
	};

	// Surface

	MeshObject<Vertex> SurfaceMeshCPU = GetParamSurfMesh( Param() );
	m_SurfaceGPU = CreateGLObjectFromMesh( SurfaceMeshCPU, vertexAttribList );

	InitSkyboxGeometry();
}

void CMyApp::InitSkyboxGeometry()
{
	// skybox geo
	MeshObject<glm::vec3> skyboxCPU =
	{
		std::vector<glm::vec3>
		{
			// hátsó lap
			glm::vec3(-1, -1, -1),
			glm::vec3(1, -1, -1),
			glm::vec3(1,  1, -1),
			glm::vec3(-1,  1, -1),
			// elülső lap
			glm::vec3(-1, -1, 1),
			glm::vec3(1, -1, 1),
			glm::vec3(1,  1, 1),
			glm::vec3(-1,  1, 1),
		},

		std::vector<GLuint>
		{
			// hátsó lap
			0, 1, 2,
			2, 3, 0,
			// elülső lap
			4, 6, 5,
			6, 4, 7,
			// bal
			0, 3, 4,
			4, 3, 7,
			// jobb
			1, 5, 2,
			5, 6, 2,
			// alsó
			1, 0, 4,
			1, 4, 5,
			// felső
			3, 2, 6,
			3, 6, 7,
		}
	};

	m_GPUSkybox = CreateGLObjectFromMesh(skyboxCPU, { { 0, offsetof(glm::vec3,x), 3, GL_FLOAT } });

}

void CMyApp::CleanSkyboxGeometry()
{
	CleanOGLObject(m_GPUSkybox);
}

void CMyApp::CleanGeometry()
{
	CleanOGLObject( m_SurfaceGPU );

	CleanSkyboxGeometry();
}

void CMyApp::InitTextures()
{
	// sampler

	glCreateSamplers( 1, &m_SamplerID );
	glSamplerParameteri( m_SamplerID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glSamplerParameteri( m_SamplerID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glSamplerParameteri( m_SamplerID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	glSamplerParameteri( m_SamplerID, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	// diffuse texture

	ImageRGBA Image = ImageFromFile( "Assets/color_checkerboard.png" );

	glCreateTextures( GL_TEXTURE_2D, 1, &m_TextureID );
	glTextureStorage2D( m_TextureID, NumberOfMIPLevels( Image ), GL_RGBA8, Image.width, Image.height );
	glTextureSubImage2D( m_TextureID, 0, 0, 0, Image.width, Image.height, GL_RGBA, GL_UNSIGNED_BYTE, Image.data() );

	glGenerateTextureMipmap( m_TextureID );

	// Earth
	Image = ImageFromFile("Assets/Earth/colorTex.jpg");
	glCreateTextures(GL_TEXTURE_2D, 1, &m_TextureColorEarthID);
	glTextureStorage2D(m_TextureColorEarthID, NumberOfMIPLevels(Image), GL_RGBA8, Image.width, Image.height);
	glTextureSubImage2D(m_TextureColorEarthID, 0, 0, 0, Image.width, Image.height, GL_RGBA, GL_UNSIGNED_BYTE, Image.data());
	glGenerateTextureMipmap(m_TextureColorEarthID);


	Image = ImageFromFile("Assets/Earth/heightMapTex.png");
	glCreateTextures(GL_TEXTURE_2D, 1, &m_TextureHeightEarthID);
	glTextureStorage2D(m_TextureHeightEarthID, NumberOfMIPLevels(Image), GL_RGBA8, Image.width, Image.height);
	glTextureSubImage2D(m_TextureHeightEarthID, 0, 0, 0, Image.width, Image.height, GL_RGBA, GL_UNSIGNED_BYTE, Image.data());
	glGenerateTextureMipmap(m_TextureHeightEarthID);

	// Sun
	Image = ImageFromFile("Assets/Sun/sunColorTex.jpg");
	glCreateTextures(GL_TEXTURE_2D, 1, &m_TextureColorSunID);
	glTextureStorage2D(m_TextureColorSunID, NumberOfMIPLevels(Image), GL_RGBA8, Image.width, Image.height);
	glTextureSubImage2D(m_TextureColorSunID, 0, 0, 0, Image.width, Image.height, GL_RGBA, GL_UNSIGNED_BYTE, Image.data());
	glGenerateTextureMipmap(m_TextureColorSunID);


	Image = ImageFromFile("Assets/Sun/sunHeightTex.png");
	glCreateTextures(GL_TEXTURE_2D, 1, &m_TextureHeightSunID);
	glTextureStorage2D(m_TextureHeightSunID, NumberOfMIPLevels(Image), GL_RGBA8, Image.width, Image.height);
	glTextureSubImage2D(m_TextureHeightSunID, 0, 0, 0, Image.width, Image.height, GL_RGBA, GL_UNSIGNED_BYTE, Image.data());
	glGenerateTextureMipmap(m_TextureHeightSunID);



	InitSkyboxTexture();
}

void CMyApp::InitSkyboxTexture()
{
	static const char* skyboxFiles[6] = {
		"Assets/SkyBox2/xpos.png",
		"Assets/SkyBox2/xneg.png",
		"Assets/SkyBox2/ypos.png",
		"Assets/SkyBox2/yneg.png",
		"Assets/SkyBox2/zpos.png",
		"Assets/SkyBox2/zneg.png",
	};

	ImageRGBA images[6];
	for (int i = 0; i < 6; ++i)
	{
		images[i] = ImageFromFile(skyboxFiles[i], false);
	}

	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_TextureSkyboxID);
	glTextureStorage2D(m_TextureSkyboxID, 1, GL_RGBA8, images[0].width, images[0].height);

	for (int face = 0; face < 6; ++face)
	{
		glTextureSubImage3D(m_TextureSkyboxID, 0, 0, 0, face, images[face].width, images[face].height, 1, GL_RGBA, GL_UNSIGNED_BYTE, images[face].data());
	}
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void CMyApp::CleanSkyboxTexture()
{
	glDeleteTextures(1, &m_TextureSkyboxID);
}

void CMyApp::CleanTextures()
{
	glDeleteTextures( 1, &m_TextureID );

	CleanSkyboxTexture();
}

bool CMyApp::Init()
{
	SetupDebugCallback();

	// törlési szín legyen kékes
	glClearColor(0.125f, 0.25f, 0.5f, 1.0f);

	glPointSize( 16.0f ); // nagyobb pontok
	glLineWidth( 4.0f ); // vastagabb vonalak

	InitShaders();
	InitGeometry();
	InitTextures();

	//
	// egyéb inicializálás
	//

	glEnable(GL_CULL_FACE); // kapcsoljuk be a hátrafelé néző lapok eldobását
	glCullFace(GL_BACK);    // GL_BACK: a kamerától "elfelé" néző lapok, GL_FRONT: a kamera felé néző lapok

	glEnable(GL_DEPTH_TEST); // mélységi teszt bekapcsolása (takarás)

	// kamera
	m_camera.SetView(
		glm::vec3(0.0, 7.0, 7.0),	// honnan nézzük a színteret	   - eye
		glm::vec3(0.0, 0.0, 0.0),   // a színtér melyik pontját nézzük - at
		glm::vec3(0.0, 1.0, 0.0));  // felfelé mutató irány a világban - up

	m_cameraManipulator.SetCamera( &m_camera );

	return true;
}

void CMyApp::Clean()
{
	CleanShaders();
	CleanGeometry();
	CleanTextures();
}

void CMyApp::Update( const SUpdateInfo& updateInfo )
{
	m_ElapsedTimeInSec = updateInfo.ElapsedTimeInSec;

	m_cameraManipulator.Update( updateInfo.DeltaTimeInSec );
	
	// kivetelesen a fényforrás a kamera pozíciója legyen, hogy mindig lássuk a feluletet,
	// es ne keljen allitgatni a fenyforrast
    m_lightPos = glm::vec4( m_camera.GetEye(), 1.0 );
}

void CMyApp::Render()
{
	// töröljük a frampuffert (GL_COLOR_BUFFER_BIT)...
	// ... és a mélységi Z puffert (GL_DEPTH_BUFFER_BIT)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//RenderEarth();

	RenderSun();

	// shader kikapcsolasa
	glUseProgram( 0 );

	// - Textúrák kikapcsolása, minden egységre külön
	glBindTextureUnit( 0, 0 );
	glBindTextureUnit( 1, 0 );
	glBindSampler( 0, 0 );

	// VAO kikapcsolása
	glBindVertexArray( 0 );

	RenderSkybox();
}

void CMyApp::RenderSun()
{
	// - Program
	glUseProgram(m_ProgramSunID);

	// - Uniform paraméterek
	// view és projekciós mátrix
	glUniformMatrix4fv(ul("viewProj"), 1, GL_FALSE, glm::value_ptr(m_camera.GetViewProj()));

	glm::mat4 matWorld = glm::identity<glm::mat4>();

	glUniformMatrix4fv(ul("world"), 1, GL_FALSE, glm::value_ptr(matWorld));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_TextureColorSunID);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_TextureHeightSunID);

	// - textúraegységek beállítása
	glUniform1i(ul("colorTexImage"), 0);
	glUniform1i(ul("heightTexImage"), 1);

	// - VAO
	glBindVertexArray(m_SurfaceGPU.vaoID);

	// Rajzolási parancs kiadása
	glDrawElements(GL_TRIANGLES,
		m_SurfaceGPU.count,
		GL_UNSIGNED_INT,
		nullptr);
}

void CMyApp::RenderEarth()
{
	// - Program
	glUseProgram(m_ProgramPlanetID);

	// - Uniform paraméterek
	// view és projekciós mátrix
	glUniformMatrix4fv(ul("viewProj"), 1, GL_FALSE, glm::value_ptr(m_camera.GetViewProj()));


	glm::mat4 matWorld = glm::translate(glm::vec3(2,0,0));

	glUniformMatrix4fv(ul("world"), 1, GL_FALSE, glm::value_ptr(matWorld));
	glUniformMatrix4fv(ul("worldIT"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(matWorld))));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_TextureColorEarthID);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_TextureHeightEarthID);

	// - textúraegységek beállítása
	glUniform1i(ul("colorTexImage"), 0);
	glUniform1i(ul("heightTexImage"), 1);

	// - Textúrák beállítása, minden egységre külön
	//glBindTextureUnit( 0, m_TextureColorEarthID);
	//glBindSampler( 0, m_SamplerID );
	//glBindTextureUnit( 1, m_TextureHeightEarthID);
	//glBindSampler( 1, m_SamplerID );

	// - VAO
	glBindVertexArray(m_SurfaceGPU.vaoID);

	SetLightning();

	// Rajzolási parancs kiadása
	glDrawElements(GL_TRIANGLES,
		m_SurfaceGPU.count,
		GL_UNSIGNED_INT,
		nullptr);
}

void CMyApp::RenderSkybox()
{
	// - VAO
	glBindVertexArray(m_GPUSkybox.vaoID);

	// - Program
	glUseProgram(m_ProgramSkyboxID);

	// - uniform parameterek
	glUniformMatrix4fv(ul("viewProj"), 1, GL_FALSE, glm::value_ptr(m_camera.GetViewProj()));
	glUniformMatrix4fv(ul("world"), 1, GL_FALSE, glm::value_ptr(glm::translate(m_camera.GetEye())));

	// - textúraegységek beállítása
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_TextureSkyboxID);
	glUniform1i(ul("skyboxTexture"), 0);

	// mentsük el az előző Z-test eredményt, azaz azt a relációt, ami alapján update-eljük a pixelt.
	GLint prevDepthFnc;
	glGetIntegerv(GL_DEPTH_FUNC, &prevDepthFnc);

	// most kisebb-egyenlőt használjunk, mert mindent kitolunk a távoli vágósíkokra
	glDepthFunc(GL_LEQUAL);

	// Rajzolási parancs kiadása
	glDrawElements(GL_TRIANGLES,
		m_GPUSkybox.count,
		GL_UNSIGNED_INT,
		nullptr);

	glDepthFunc(prevDepthFnc);

	// shader kikapcsolasa
	glUseProgram(0);

	// - Textúrák kikapcsolása, minden egységre külön
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void CMyApp::SetLightning()
{
	// - Fényforrások beállítása
	glUniform3fv(ul("cameraPos"), 1, glm::value_ptr(m_camera.GetEye()));
	glUniform4fv(ul("lightPos"), 1, glm::value_ptr(m_lightPos));

	glUniform3fv(ul("La"), 1, glm::value_ptr(m_La));
	glUniform3fv(ul("Ld"), 1, glm::value_ptr(m_Ld));
	glUniform3fv(ul("Ls"), 1, glm::value_ptr(m_Ls));

	glUniform1f(ul("lightConstantAttenuation"), m_lightConstantAttenuation);
	glUniform1f(ul("lightLinearAttenuation"), m_lightLinearAttenuation);
	glUniform1f(ul("lightQuadraticAttenuation"), m_lightQuadraticAttenuation);

	// - Anyagjellemzők beállítása
	glUniform3fv(ul("Ka"), 1, glm::value_ptr(m_Ka));
	glUniform3fv(ul("Kd"), 1, glm::value_ptr(m_Kd));
	glUniform3fv(ul("Ks"), 1, glm::value_ptr(m_Ks));

	glUniform1f(ul("Shininess"), m_Shininess);
}

void CMyApp::RenderGUI()
{
	//ImGui::ShowDemoWindow();
	if ( ImGui::Begin( "Lighting settings" ) )
	{		
		ImGui::InputFloat("Shininess", &m_Shininess, 0.1f, 1.0f, "%.1f" );
		static float Kaf = 1.0f;
		static float Kdf = 1.0f;
		static float Ksf = 1.0f;
		if ( ImGui::SliderFloat( "Ka", &Kaf, 0.0f, 1.0f ) )
		{
			m_Ka = glm::vec3( Kaf );
		}
		if ( ImGui::SliderFloat( "Kd", &Kdf, 0.0f, 1.0f ) )
		{
			m_Kd = glm::vec3( Kdf );
		}
		if ( ImGui::SliderFloat( "Ks", &Ksf, 0.0f, 1.0f ) )
		{
			m_Ks = glm::vec3( Ksf );
		}

		{
			static glm::vec2 lightPosXZ = glm::vec2( 0.0f );
			lightPosXZ = glm::vec2( m_lightPos.x, m_lightPos.z );
			if ( ImGui::SliderFloat2( "Light Position XZ", glm::value_ptr( lightPosXZ ), -1.0f, 1.0f ) )
			{
				float lightPosL2 = lightPosXZ.x * lightPosXZ.x + lightPosXZ.y * lightPosXZ.y;
				if ( lightPosL2 > 1.0f ) // Ha kívülre esne a körön, akkor normalizáljuk
				{
					lightPosXZ /= sqrtf( lightPosL2 );
					lightPosL2 = 1.0f;
				}

				m_lightPos.x = lightPosXZ.x;
				m_lightPos.z = lightPosXZ.y;
				m_lightPos.y = sqrtf( 1.0f - lightPosL2 );
			}
			ImGui::LabelText( "Light Position Y", "%f", m_lightPos.y );
		}
	}
	ImGui::End();
}

// https://wiki.libsdl.org/SDL2/SDL_KeyboardEvent
// https://wiki.libsdl.org/SDL2/SDL_Keysym
// https://wiki.libsdl.org/SDL2/SDL_Keycode
// https://wiki.libsdl.org/SDL2/SDL_Keymod

void CMyApp::KeyboardDown(const SDL_KeyboardEvent& key)
{	
	if ( key.repeat == 0 ) // Először lett megnyomva
	{
		if ( key.keysym.sym == SDLK_F5 && key.keysym.mod & KMOD_CTRL )
		{
			CleanShaders();
			InitShaders();
		}
		if ( key.keysym.sym == SDLK_F1 )
		{
			GLint polygonModeFrontAndBack[ 2 ] = {};
			// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGet.xhtml
			glGetIntegerv( GL_POLYGON_MODE, polygonModeFrontAndBack ); // Kérdezzük le a jelenlegi polygon módot! Külön adja a front és back módokat.
			GLenum polygonMode = ( polygonModeFrontAndBack[ 0 ] != GL_FILL ? GL_FILL : GL_LINE ); // Váltogassuk FILL és LINE között!
			// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glPolygonMode.xhtml
			glPolygonMode( GL_FRONT_AND_BACK, polygonMode ); // Állítsuk be az újat!
		}
	}
	m_cameraManipulator.KeyboardDown( key );
}

void CMyApp::KeyboardUp(const SDL_KeyboardEvent& key)
{
	m_cameraManipulator.KeyboardUp( key );
}

// https://wiki.libsdl.org/SDL2/SDL_MouseMotionEvent

void CMyApp::MouseMove(const SDL_MouseMotionEvent& mouse)
{
	m_cameraManipulator.MouseMove( mouse );
}

// https://wiki.libsdl.org/SDL2/SDL_MouseButtonEvent

void CMyApp::MouseDown(const SDL_MouseButtonEvent& mouse)
{
}

void CMyApp::MouseUp(const SDL_MouseButtonEvent& mouse)
{
}

// https://wiki.libsdl.org/SDL2/SDL_MouseWheelEvent

void CMyApp::MouseWheel(const SDL_MouseWheelEvent& wheel)
{
	m_cameraManipulator.MouseWheel( wheel );
}


// a két paraméterben az új ablakméret szélessége (_w) és magassága (_h) található
void CMyApp::Resize(int _w, int _h)
{
	glViewport(0, 0, _w, _h);
	m_camera.SetAspect( static_cast<float>(_w) / _h );
}

// Le nem kezelt, egzotikus esemény kezelése
// https://wiki.libsdl.org/SDL2/SDL_Event

void CMyApp::OtherEvent( const SDL_Event& ev )
{

}

