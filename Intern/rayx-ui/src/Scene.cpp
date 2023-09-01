#include "Scene.h"

#include "Beamline/OpticalElement.h"
#include "Debug/Debug.h"

Scene::Scene(Device& device) : m_Device(device) {}

void Scene::setup(const RAYX::RenderObjectVec& renderObjects, const RAYX::BundleHistory& bundleHistory) {
    for (const auto& renderObject : renderObjects) {
        fromRenderObject(renderObject);
    }

    // Event types:
    // const double ETYPE_FLY_OFF = 0;       --> World coordinates
    // const double ETYPE_JUST_HIT_ELEM = 1; --> Element coordinates
    // const double ETYPE_ABSORBED = 3;      --> Element coordinates
    // Rest are error codes
    glm::vec3 yellow = {1.0f, 1.0f, 0.0f};
    glm::vec3 blue = {0.0f, 0.0f, 1.0f};
    for (const auto& rayHist : bundleHistory) {
        glm::vec3 rayLastPos = {0.0f, 0.0f, 0.0f};
        for (const auto& event : rayHist) {
            if (event.m_eventType == ETYPE_JUST_HIT_ELEM || event.m_eventType == ETYPE_ABSORBED) {
                // Events where rays hit objects are in element coordinates
                // We need to convert them to world coordinates
                glm::vec4 lastElementPos = renderObjects[(size_t)event.m_lastElement].position;
                glm::mat4 lastElementOri = renderObjects[(size_t)event.m_lastElement].orientation;
                glm::dmat4 outTrans = RAYX::calcTransformationMatrices(lastElementPos, lastElementOri, false);
                glm::vec4 worldPos = outTrans * glm::vec4(event.m_position, 1.0f);

                Vertex origin = {{rayLastPos.x, rayLastPos.y, rayLastPos.z}, yellow};
                Vertex point = {{worldPos.x, worldPos.y, worldPos.z}, yellow};
                addLine(origin, point);

                rayLastPos = point.pos;
            } else if (event.m_eventType == ETYPE_FLY_OFF) {
                // The origin here is the position of the event
                // And the point towards the direction of the ray
                // This ray flies off into infinity, so we need to calculate a sensible
                glm::vec3 eventPos = event.m_position;
                glm::vec3 eventDir = event.m_direction;
                glm::vec3 pointPos = eventPos + eventDir * 1000.0f;

                Vertex origin = {{eventPos.x, eventPos.y, eventPos.z}, blue};
                Vertex point = {{pointPos.x, pointPos.y, pointPos.z}, blue};
                // addLine(origin, point);
            }
        }
    }

    updateBuffers();
}

void Scene::addTriangle(const Vertex v1, const Vertex v2, const Vertex v3) {
    addVertex(v1, TRIA_TOPOGRAPHY);
    addVertex(v2, TRIA_TOPOGRAPHY);
    addVertex(v3, TRIA_TOPOGRAPHY);
}

void Scene::addLine(const Vertex v1, const Vertex v2) {
    addVertex(v1, LINE_TOPOGRAPHY);
    addVertex(v2, LINE_TOPOGRAPHY);
}

void Scene::fromRenderObject(const RAYX::RenderObject& renderObject) {
    // Define your base colors
    glm::vec4 white = {1.0f, 1.0f, 1.0f, 1.0f};
    glm::vec4 black = {0.0f, 0.0f, 0.0f, 1.0f};
    glm::vec4 blueBase = {0.0f, 0.0f, 1.0f, 1.0f};
    glm::vec4 redBase = {1.0f, 0.0f, 0.0f, 1.0f};
    glm::vec4 greenBase = {0.0f, 1.0f, 0.0f, 1.0f};
    // Define the colors of the corners of the triangles
    glm::vec4 tl, tr, bl, br;
    // Define color variations based on object type
    if (renderObject.type == 0) {               // Plane Mirror is green
        tl = glm::mix(greenBase, white, 0.4f);  // make it 40% lighter
        tr = greenBase;
        bl = greenBase;
        br = glm::mix(greenBase, black, 0.6f);  // make it 60% darker
    } else if (renderObject.type == 10) {       // Image plane is blue
        tl = glm::mix(blueBase, white, 0.4f);   // make it 40% lighter
        tr = blueBase;
        bl = blueBase;
        br = glm::mix(blueBase, black, 0.6f);  // make it 60% darker
    } else {                                   // rest is red for now
        tl = glm::mix(redBase, white, 0.4f);   // make it 40% lighter
        tr = redBase;
        bl = redBase;
        br = glm::mix(redBase, black, 0.6f);  // make it 60% darker
    }

    glm::vec4 topLeft, topRight, bottomLeft, bottomRight;
    if (renderObject.cutout.m_type == 2) {  // trapzoid
        TrapezoidCutout trapez = deserializeTrapezoid(renderObject.cutout);

        topLeft = {-trapez.m_sizeA_x1 / 2.0f, 0, trapez.m_size_x2 / 2.0f, 1.0f};
        topRight = {trapez.m_sizeA_x1 / 2.0f, 0, trapez.m_size_x2 / 2.0f, 1.0f};
        bottomLeft = {-trapez.m_sizeB_x1 / 2.0f, 0, -trapez.m_size_x2 / 2.0f, 1.0f};
        bottomRight = {trapez.m_sizeB_x1 / 2.0f, 0, -trapez.m_size_x2 / 2.0f, 1.0f};
    } else {  // rectangle, unlimited, elliptical (treat all as rectangles)

        double width, height;
        if (renderObject.cutout.m_type == 3 || renderObject.cutout.m_type == 1) {  // unlimited or elliptical
            width = 100.0f;
            height = 100.0f;
        } else {  // rectangle
            RectCutout rect = deserializeRect(renderObject.cutout);
            width = rect.m_size_x1;
            height = rect.m_size_x2;
        }

        // Calculate two triangle from the position, orientation and size of the render object
        // Assume the rectangle is in the XZ plane, and its center is at position.
        // First, calculate the corners of the rectangle in the model's local space
        topLeft = {-width / 2.0f, 0, height / 2.0f, 1.0f};
        topRight = {width / 2.0f, 0, height / 2.0f, 1.0f};
        bottomLeft = {-width / 2.0f, 0, -height / 2.0f, 1.0f};
        bottomRight = {width / 2.0f, 0, -height / 2.0f, 1.0f};
    }

    // To world coordinates
    glm::vec4 worldTopLeft = renderObject.orientation * topLeft + renderObject.position;
    glm::vec4 worldTopRight = renderObject.orientation * topRight + renderObject.position;
    glm::vec4 worldBottomLeft = renderObject.orientation * bottomLeft + renderObject.position;
    glm::vec4 worldBottomRight = renderObject.orientation * bottomRight + renderObject.position;

    // Create the vertices
    Vertex v1 = {{worldTopLeft.x, worldTopLeft.y, worldTopLeft.z}, tl};
    Vertex v2 = {{worldTopRight.x, worldTopRight.y, worldTopRight.z}, tr};
    Vertex v3 = {{worldBottomLeft.x, worldBottomLeft.y, worldBottomLeft.z}, bl};
    Vertex v4 = {{worldBottomRight.x, worldBottomRight.y, worldBottomRight.z}, br};
    addTriangle(v1, v2, v3);
    addTriangle(v2, v3, v4);
}

void Scene::draw(VkCommandBuffer commandBuffer, Topography topography) const {
    if (m_indexBuffers[topography] != nullptr) {
        vkCmdDrawIndexed(commandBuffer, (uint32_t)m_indices[topography].size(), 1, 0, 0, 0);
    } else {
        vkCmdDraw(commandBuffer, (uint32_t)m_vertices.size(), 1, 0, 0);
    }
}

void Scene::bind(VkCommandBuffer commandBuffer, Topography topography) const {
    VkBuffer buffers[] = {m_vertexBuffer->getBuffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

    if (m_indexBuffers[topography] != nullptr) {
        vkCmdBindIndexBuffer(commandBuffer, m_indexBuffers[topography]->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
    }
}

// Function that adds vertex to scene if it doesn't exist, otherwise adds index of existing vertex
uint32_t Scene::addVertex(const Vertex v, Topography topography) {
    auto index = vertexExists(v);
    if (index.has_value()) {
        m_indices[topography].push_back(index.value());
        return index.value();
    } else {
        m_vertices.push_back(v);
        m_indices[topography].push_back((uint32_t)m_vertices.size() - 1);
        return (uint32_t)m_vertices.size() - 1;
    }
}

// Function that returns index of vertex if it exists, otherwise returns none
std::optional<uint32_t> Scene::vertexExists(const Vertex v) const {
    for (int i = 0; i < m_vertices.size(); i++) {
        if (m_vertices[i].pos == v.pos && m_vertices[i].color == v.color) {
            return i;
        }
    }
    return std::nullopt;
}

void Scene::updateBuffers() {
    updateVertexBuffer();
    updateIndexBuffers();
}

void Scene::updateVertexBuffer() {
    // update the vertex buffer
    m_vertexBuffer = std::make_unique<Buffer>(m_Device, sizeof(Vertex), (uint32_t)m_vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    m_vertexBuffer->map();
    m_vertexBuffer->writeToBuffer(m_vertices.data(), m_vertices.size() * sizeof(Vertex));
    m_vertexBuffer->flush();
}

void Scene::updateIndexBuffers() {
    // update the index buffers
    m_indexBuffers[0] = std::make_unique<Buffer>(m_Device, sizeof(uint32_t), (uint32_t)m_indices[0].size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    m_indexBuffers[0]->map();
    m_indexBuffers[0]->writeToBuffer(m_indices[0].data(), m_indices[0].size() * sizeof(uint32_t));
    m_indexBuffers[0]->flush();

    m_indexBuffers[1] = std::make_unique<Buffer>(m_Device, sizeof(uint32_t), (uint32_t)m_indices[1].size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    m_indexBuffers[1]->map();
    m_indexBuffers[1]->writeToBuffer(m_indices[1].data(), m_indices[1].size() * sizeof(uint32_t));
    m_indexBuffers[1]->flush();
}