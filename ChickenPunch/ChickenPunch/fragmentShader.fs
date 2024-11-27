#version 330 core

struct Material {
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;  
in vec3 Normal;  
in vec4 Color;
in vec2 TexCoords;
in float UseTexture;

out vec4 FragColor;

uniform vec3 viewPos;
uniform DirLight dirLight;
uniform Material material;

uniform sampler2D texture_diffuse1;

// Función para calcular la iluminación direccional
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // Cálculo de iluminación direccional
    vec3 lightingResult = CalcDirLight(dirLight, norm, viewDir);

    if (int(UseTexture) == 0)
    {
        // Usar el color directo si no hay textura
        FragColor = vec4(lightingResult, 1.0) * Color;
    }
    else
    {
        // Usar la textura si está habilitada
        vec4 texColor = texture(texture_diffuse1, TexCoords);
        FragColor = vec4(lightingResult, 1.0) * texColor;
    }
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    // Dirección de la luz
    vec3 lightDir = normalize(-light.direction);

    // Cálculo de componentes de iluminación
    float diff = max(dot(normal, lightDir), 0.0);  // Difusa
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);  // Especular

    vec3 ambient = light.ambient * material.diffuse;  // Ambiental
    vec3 diffuse = light.diffuse * diff * material.diffuse;  // Difusa
    vec3 specular = light.specular * spec * material.specular;  // Especular

    return ambient + diffuse + specular;
}
