#include "platform.h"

#include <SDL_events.h>
#ifdef WIN32
#include <codecvt>
#else
#include <unistd.h>
#endif
#include <fcntl.h>

#include <fcntl.h>
#include "utils/FileSystemUtil.h"
#include "utils/StringUtil.h"
#include "utils/TimeUtil.h"
#include <Windows.h>
#include <atlbase.h>
#include <iostream>
#include "Log.h"
#include <tchar.h>
#include <iostream>
#include <string>
#include <algorithm>

int runShutdownCommand()
{
#ifdef WIN32 // windows
	return system("shutdown -s -t 0");
#else // osx / linux
	return system("sudo shutdown -h now");
#endif
}

int runRestartCommand()
{
#ifdef WIN32 // windows
	return system("shutdown -r -t 0");
#else // osx / linux
	return system("sudo shutdown -r now");
#endif
}

int runSystemCommand(const std::string& cmd_utf8)
{
#ifdef WIN32
	// on Windows we use _wsystem to support non-ASCII paths
	// which requires converting from utf8 to a wstring
	typedef std::codecvt_utf8<wchar_t> convert_type;
	std::wstring_convert<convert_type, wchar_t> converter;
	std::wstring wchar_str = converter.from_bytes(cmd_utf8);
	return _wsystem(wchar_str.c_str());
#else
	return system(cmd_utf8.c_str());
#endif
}


// ROTINA CRIADA POR RODRIGO A. MELO
// PARA MS WINDOWS X86 (32-BITS)
// ESTA ROTINA NÃO CRIA UMA CHAMADA DE CONSOLE VISÍVEL PARA O USUÁRIO
// LIMITAÇÕES: TODA A ESTRUTURA DE PASTAS ATÉ O PRIMEIRO EXECUTAEL NÃO PODE CONTER NOMES COMPOSTOS
// OU CARACTERES ESPECIAIS
int runWSystemCommand(const std::string& cmd_utf8) {
#ifdef WIN32
	STARTUPINFO si;  // ARMAZENA AS INFORMACOES DE INICIO DO PROCESSO
	PROCESS_INFORMATION pi; // RESPONSAVEL POR ARMAZENAR O STATUS DO PROCESSO

	// ALOCA MEMORIA
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	// RECEBE O COMANDO A SER EXECUTADO PELO SISTEMA
	std::string s = cmd_utf8;

	// CRIA UM REGISTRO NO LOG
	LOG(LogInfo) << "	" << "CONSOLE ROM PATH";
	LOG(LogInfo) << "	" << (TCHAR*)s.c_str();

	// INICIA PROCESSO FILHO. 
	BOOL result = CreateProcess(
		NULL,
		(TCHAR*)s.c_str(),
		NULL,
		NULL,
		FALSE,
		NULL,
		NULL,
		NULL,
		&si,
		&pi);

	if (!result)
	{
		// SE NAO HOUVER RESULTADO, NÓS FALHAMOS.
		return 1;
	}
	else
	{
		// PROCESSO CRIADO, AGUARDANDO FINALIZAÇÃO.
		WaitForSingleObject(pi.hProcess, INFINITE);

		// PEGA O CÓDIGO DE SAÍDA.
		DWORD exitCode;
		result = GetExitCodeProcess(pi.hProcess, &exitCode);

		// FECHA OS CONTROLES.
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		if (!result)
		{
			// NÃO FOI POSSÍVEL PEGAR O CÓDIGO DE SAÍDA.
			return 1;
		}


		// CONSEGUIMOS!
		return 0;
	}
#else
	return system(cmd_utf8.c_str());
#endif
}


int quitES(const std::string& filename)
{
	touch(filename);
	SDL_Event* quit = new SDL_Event();
	quit->type = SDL_QUIT;
	SDL_PushEvent(quit);
	return 0;
}

void touch(const std::string& filename)
{
#ifdef WIN32
	FILE* fp = fopen(filename.c_str(), "ab+");
	if (fp != NULL)
		fclose(fp);
#else
	int fd = open(filename.c_str(), O_CREAT|O_WRONLY, 0644);
	if (fd >= 0)
		close(fd);
#endif
}