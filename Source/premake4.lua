
project "DevelopmentGame"
	kind "WindowedApp"
	language "C++"
	files { "./AutoGenRegisterReflection.cpp", "./**.h", "./**.inl", "./**.cpp" }
   debugdir "../Dist"
   includedirs { 
      "./", 
      psybrusSDK .. "/Engine/Source/Shared/", 
      psybrusSDK .. "/Engine/Source/Platforms/Windows/"
   }
	-- External includes.
	includedirs { 
      psybrusSDK .. "/External/jsoncpp/include", 
      boostInclude
   }

	configuration "windows"
         prebuildcommands {
               "C:\\Python27\\python.exe $(PSYBRUS_SDK)/reflection_parse.py DevelopmentGame"
         }

         libdirs {
            openal32LibPath,
            boostLib
         }
         links {
            -- Windows libs.
            "user32",
            "gdi32",
            "opengl32",
            "winmm",
            "ws2_32",
            "IPHlpApi",
            "OpenAL32",

            -- Engine libs.
            "Engine",
         }

