#version 330 core

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

uniform bool useLighting;

void main(){
    if(!useLighting){
        FragColor = vec4(objectColor, 1.0);
    }else{
        //Ambient
        float ambientStrength = 0.1;
        vec3 ambient = ambientStrength * lightColor;

        //diffuse
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;

        vec3 result = (ambient + diffuse) * objectColor;
        FragColor = vec4(result, 1);
    }
}