#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec4 aColor;
layout (location = 3) in vec2 aTexCoords;

out vec3 Normal;      // Normales para iluminaci�n
out vec3 FragPos;     // Posici�n del fragmento para iluminaci�n
out vec2 TexCoords;   // Coordenadas de textura
out vec4 Color;       // Color del v�rtice
out float UseTexture; // Indicador de uso de textura

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Transformaciones de posici�n
    gl_Position = projection * view * model * vec4(aPos, 1.0);

    // Posici�n del fragmento en el espacio mundial
    FragPos = vec3(model * vec4(aPos, 1.0));

    // Normales transformadas al espacio mundial
    Normal = mat3(transpose(inverse(model))) * aNormal;

    // Color y coordenadas de textura
    TexCoords = aTexCoords;   
    Color = aColor;

    // Indicador para decidir si usar textura (se puede ajustar externamente)
    UseTexture = 0.0f; 
}
