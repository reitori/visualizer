#include "OpenGL/Scene/Detector.h"

namespace viz{
    std::vector<SimpleVertex> ChipVertices = {
        // Front Vertices //Color
        {glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec4(0.3921f, 0.3921f, 0.3921f, 1.0f)},
        {glm::vec3(1.0f, -1.0f,  1.0f), glm::vec4(0.3921f, 0.3921f, 0.3921f, 1.0f)},
        {glm::vec3(1.0f,  1.0f,  1.0f), glm::vec4(0.3921f, 0.3921f, 0.3921f, 1.0f)},
        {glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec4(0.3921f, 0.3921f, 0.3921f, 1.0f)},
        // Back Vertices  //Color
        {glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec4(0.3921f, 0.3921f, 0.3921f, 1.0f)},
        {glm::vec3(1.0f, -1.0f, -1.0f), glm::vec4(0.3921f, 0.3921f, 0.3921f, 1.0f)},
        {glm::vec3(1.0f,  1.0f, -1.0f), glm::vec4(0.3921f, 0.3921f, 0.3921f, 1.0f)},
        {glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec4(0.3921f, 0.3921f, 0.3921f, 1.0f)}
    };

    std::vector<SimpleVertex> HitVertices = {
        // Front Vertices //Color
        {glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec4(1.0f, 0.2509f, 0.0235f, 0.95f)},
        {glm::vec3(1.0f, -1.0f,  1.0f), glm::vec4(1.0f, 0.2509f, 0.0235f, 0.95f)},
        {glm::vec3(1.0f,  1.0f,  1.0f), glm::vec4(1.0f, 0.2509f, 0.0235f, 0.95f)},
        {glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec4(1.0f, 0.2509f, 0.0235f, 0.95f)},
        // Back Vertices  //Color
        {glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec4(1.0f, 0.2509f, 0.0235f, 0.95f)},
        {glm::vec3(1.0f, -1.0f, -1.0f), glm::vec4(1.0f, 0.2509f, 0.0235f, 0.95f)},
        {glm::vec3(1.0f,  1.0f, -1.0f), glm::vec4(1.0f, 0.2509f, 0.0235f, 0.95f)},
        {glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec4(1.0f, 0.2509f, 0.0235f, 0.95f)}
    };

    std::vector<GLuint> CubeIndices = {
        // front
        0, 1, 2,
        2, 3, 0,
        // right
        1, 5, 6,
        6, 2, 1,
        // back
        7, 6, 5,
        5, 4, 7,
        // left
        4, 0, 3,
        3, 7, 4,
        // bottom
        4, 5, 1,
        1, 0, 4,
        // top
        3, 2, 6,
        6, 7, 3
    };
    
    Detector::Detector(){
        ChipMesh = SimpleMesh(ChipVertices, CubeIndices, true);
        HitMesh = SimpleMesh(HitVertices, CubeIndices, true);
        m_cli = nullptr;
    }

    void Detector::init(const VisualizerCli& cli){
        m_cli = &cli;

        for(int i = 0; i < cli.getSize(); i++) {
            const json& config = cli.getConfig(i);
            std::string name = config["name"].get<std::string>();
            std::vector<float> pos = config["position"].get<std::vector<float>>();
            std::vector<float> angle = config["angle"].get<std::vector<float>>();
            std::vector<float> size = config["size"].get<std::vector<float>>();
            std::vector<float> rowcol = config["rowcol"].get<std::vector<float>>();

            Chip tempChip;
            tempChip.name = name;
            tempChip.fe_id = i;
            tempChip.pos = glm::vec3(pos[0], pos[1], pos[2]);
            tempChip.eulerRot = glm::vec3(angle[0], angle[1], angle[2]);
            tempChip.scale = glm::vec3(size[0] / 2.0f, size[1] / 2.0f, size[2] / 2.0f);
            tempChip.maxRows = rowcol[0]; tempChip.maxCols = rowcol[1];
            tempChip.hits = 0;

            glm::mat4 tempTfm = transform(tempChip.scale, tempChip.eulerRot, tempChip.pos, false);
            
            m_chips.push_back(tempChip);
            m_chipTransforms.push_back(tempTfm);
            m_chipColors.push_back(glm::vec4(0.3921f, 0.3921f, 0.3921f, 1.0f));
        };
        ChipMesh.setInstances(m_chips.size(), m_chipTransforms, m_chipColors);

        m_appLogger->info("Detector initialized with {0} chips", m_chipTransforms.size());
    }

    void Detector::update(){
        m_nfe = 0;
        m_size = 0;

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        for(int i = 0; i < m_cli->getSize(); i++) {
            

            std::unique_ptr<std::vector<pixelHit>> data = m_cli->getData(i, true);
            glm::vec3 chipScale = m_chips[i].scale;
            if(data){
                m_size += data->size();
                Chip* currChip = &m_chips[i];
                
                for(int j = 0; j < data->size(); j++){
                    std::uint16_t row = (*data)[j].row;
                    std::uint16_t col = (*data)[j].col;

                    float diffx = (2.0f * ((float)row / (float)currChip->maxRows) - 1.0f) * currChip->scale[0];
                    float diffy = (2.0f * ((float)col / (float)currChip->maxCols) - 1.0f) * currChip->scale[1];
                    glm::vec3 posRelToChip =  glm::vec3(diffx, diffy, currChip->scale[2]);
                    glm::vec3 pos = currChip->pos + glm::toMat3(glm::quat(glm::vec3(viz_TO_RADIANS(currChip->eulerRot[0]), viz_TO_RADIANS(currChip->eulerRot[1]), viz_TO_RADIANS(currChip->eulerRot[2])))) * posRelToChip;

                    m_hitTransforms.push_back(transform(hitScale, currChip->eulerRot, pos));
                    m_hitColors.push_back(glm::vec4(1.0f, 0.2509f, 0.0235f, 0.95f));

                    currChip->hits += 1;

                    ParticleHit hitEvent(currChip->name, currChip->hits, row, col);
                    eventCallback(hitEvent);
                }

                data.reset();
                m_nfe++;
            }
        }
        HitMesh.setInstances(m_hitTransforms.size(), m_hitTransforms, m_hitColors);
    }

    void Detector::render(const Shader& shader) const{
        ChipMesh.render(shader);
        HitMesh.render(shader);
    }

    glm::mat4 Detector::transform(glm::vec3 scale, glm::vec3 eulerRot, glm::vec3 pos, bool isInRadians){
        glm::mat4 tempTfm = glm::mat4(1.0f);
        tempTfm = glm::scale(tempTfm, scale);

        if(!isInRadians)
            eulerRot = glm::vec3(viz_TO_RADIANS(eulerRot[0]), viz_TO_RADIANS(eulerRot[1]), viz_TO_RADIANS(eulerRot[2]));

        tempTfm = glm::toMat4(glm::quat(eulerRot)) * tempTfm;
        tempTfm = glm::translate(glm::mat4(1.0f), pos) * tempTfm;
        return tempTfm;
    }
}