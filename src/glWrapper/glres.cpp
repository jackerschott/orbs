#include "glWrapper/glres.hpp"

namespace gl {
	bool getLastShaderError(GLuint shader, GLuint flag, std::string* errMsg)
	{
#define MSG_BUF_SIZE 1024
		GLint success;
		char log[MSG_BUF_SIZE];
		int logLength;

		glGetShaderiv(shader, flag, &success);
		if (success == false) {
			glGetShaderInfoLog(shader, sizeof(log), &logLength, log);
      if (errMsg != nullptr) {
        if (*errMsg != "" && (*errMsg)[0] != '\n')
          *errMsg += "\n" + std::string(log, logLength);
        else *errMsg += std::string(log, logLength);
      }
		}

		return success;
	}
	bool getLastProgramError(GLuint program, GLuint flag, std::string* errMsg)
	{
#define MSG_BUF_SIZE 1024
		GLint success;
		char log[MSG_BUF_SIZE];
		int logLength;

		glGetProgramiv(program, flag, &success);
		if (success == false) {
			glGetProgramInfoLog(program, sizeof(log), &logLength, log);
      if (errMsg != nullptr) {
        if (*errMsg != "" && (*errMsg)[0] != '\n')
          *errMsg += "\n" + std::string(log, logLength);
        else *errMsg += std::string(log, logLength);
      }
		}

		return success;
	}
}