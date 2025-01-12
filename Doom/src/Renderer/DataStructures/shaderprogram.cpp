﻿#include "pch.h"

#include "shaderprogram.h"


//Procedura wczytuje plik do tablicy znaków.
char* ShaderProgram::readFile(const char* fileName) {
	long long filesize;
	FILE *plik;
	char* result;

	#pragma warning(suppress : 4996) //Wyłączenie błędu w Visual Studio wynikające z nietrzymania się standardów przez Microsoft.
	plik=fopen(fileName,"rb");
	if (plik != NULL) {
		fseek(plik, 0, SEEK_END);
		filesize = ftell(plik);
		fseek(plik, 0, SEEK_SET);
		result = new char[filesize + 1];
		#pragma warning(suppress : 6386) //Wyłączenie błędu w Visual Studio wynikającego z błędnej analizy statycznej kodu.
		size_t readsize=fread(result, 1, filesize, plik);
		result[filesize] = 0;
		fclose(plik);

		return result;
	}

	return NULL;

}

//Metoda wczytuje i kompiluje shader, a następnie zwraca jego uchwyt
GLuint ShaderProgram::loadShader(GLenum shaderType,const char* fileName) {
	//Wygeneruj uchwyt na shader
	GLuint shader=glCreateShader(shaderType);//shaderType to GL_VERTEX_SHADER, GL_GEOMETRY_SHADER lub GL_FRAGMENT_SHADER
	//Wczytaj plik ze źródłem shadera do tablicy znaków
	const GLchar* shaderSource=readFile(fileName);
	//Powiąż źródło z uchwytem shadera
	glShaderSource(shader,1,&shaderSource,NULL);
	//Skompiluj źródło
	glCompileShader(shader);
	//Usuń źródło shadera z pamięci (nie będzie już potrzebne)
	delete []shaderSource;

	//Pobierz log błędów kompilacji i wyświetl
	int infologLength = 0;
	int charsWritten  = 0;
	char *infoLog;

	glGetShaderiv(shader, GL_INFO_LOG_LENGTH,&infologLength);

	if (infologLength > 1) {
		infoLog = new char[infologLength];
		glGetShaderInfoLog(shader, infologLength, &charsWritten, infoLog);
		LOGINFO(infoLog);
		delete []infoLog;
	}

	//Zwróć uchwyt wygenerowanego shadera
	return shader;
}

ShaderProgram::ShaderProgram(const std::string& name, const char* vertexShaderFile,const char* geometryShaderFile,const char* fragmentShaderFile) 
	: LibraryElement(name)
{
	//Wczytaj vertex shader
	LOGTRACE("Loading vertex shader...");
	vertexShader=loadShader(GL_VERTEX_SHADER,vertexShaderFile);

	//Wczytaj geometry shader
	if (geometryShaderFile!=NULL) {
		LOGTRACE("Loading geometry shader...");
		geometryShader=loadShader(GL_GEOMETRY_SHADER,geometryShaderFile);
	} else {
		geometryShader=0;
	}

	//Wczytaj fragment shader
	LOGTRACE("Loading fragment shader...");
	fragmentShader=loadShader(GL_FRAGMENT_SHADER,fragmentShaderFile);

	//Wygeneruj uchwyt programu cieniującego
	shaderProgram=glCreateProgram();

	//Podłącz do niego shadery i zlinkuj program
	glAttachShader(shaderProgram,vertexShader);
	glAttachShader(shaderProgram,fragmentShader);
	if (geometryShaderFile!=NULL) glAttachShader(shaderProgram,geometryShader);
	glLinkProgram(shaderProgram);

	//Pobierz log błędów linkowania i wyświetl
	int infologLength = 0;
	int charsWritten  = 0;
	char *infoLog;

	glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH,&infologLength);

	if (infologLength > 1)
	{
		infoLog = new char[infologLength];
		glGetProgramInfoLog(shaderProgram, infologLength, &charsWritten, infoLog);
		LOGINFO("%s\n",infoLog);
		delete []infoLog;
	}

	LOGINFO("Shader program created ");
}

ShaderProgram::~ShaderProgram()
{
	//Odłącz shadery od programu
	glDetachShader(shaderProgram, vertexShader);
	if (geometryShader != 0) glDetachShader(shaderProgram, geometryShader);
	glDetachShader(shaderProgram, fragmentShader);

	//Wykasuj shadery
	glDeleteShader(vertexShader);
	if (geometryShader != 0) glDeleteShader(geometryShader);
	glDeleteShader(fragmentShader);

	//Wykasuj program
	glDeleteProgram(shaderProgram);
}

ShaderProgram::ShaderProgram(ShaderProgram&& shaderProgram) noexcept
	: LibraryElement(shaderProgram.GetName()), shaderProgram(shaderProgram.shaderProgram),
	vertexShader(shaderProgram.vertexShader), geometryShader(shaderProgram.geometryShader),
	fragmentShader(shaderProgram.fragmentShader)
{
	shaderProgram.shaderProgram = 0;
	shaderProgram.vertexShader = 0;
	shaderProgram.geometryShader = 0;
	shaderProgram.fragmentShader = 0;
}

//Włącz używanie programu cieniującego reprezentowanego przez aktualny obiekt
void ShaderProgram::use() const {
	glUseProgram(shaderProgram);
}

//Pobierz numer slotu odpowiadającego zmiennej jednorodnej o nazwie variableName
GLuint ShaderProgram::u(const char* variableName) const {
	return glGetUniformLocation(shaderProgram,variableName);
}

//Pobierz numer slotu odpowiadającego atrybutowi o nazwie variableName
GLuint ShaderProgram::a(const char* variableName) const {
	return glGetAttribLocation(shaderProgram,variableName);
}
