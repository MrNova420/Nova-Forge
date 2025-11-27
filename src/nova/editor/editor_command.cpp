/**
 * @file editor_command.cpp
 * @brief NovaCore Editor™ - Command Pattern Implementation
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 */

#include <nova/editor/editor_command.hpp>
#include <nova/editor/editor_context.hpp>

namespace nova::editor {

// ============================================================================
// MoveCommand Implementation
// ============================================================================

MoveCommand::MoveCommand(ecs::Entity entity, const Vec3& oldPosition, const Vec3& newPosition) {
    m_moves.push_back({entity, oldPosition, newPosition});
}

MoveCommand::MoveCommand(const std::vector<ecs::Entity>& entities,
                         const std::vector<Vec3>& oldPositions,
                         const std::vector<Vec3>& newPositions) {
    m_moves.reserve(entities.size());
    for (usize i = 0; i < entities.size(); ++i) {
        m_moves.push_back({
            entities[i],
            oldPositions[i],
            newPositions[i]
        });
    }
}

void MoveCommand::execute() {
    auto* world = EditorContext::get().getWorld();
    if (!world) return;
    
    for (const auto& move : m_moves) {
        // Note: Would set transform position on entity
        // world->getComponent<TransformComponent>(move.entity)->position = move.newPosition;
    }
}

void MoveCommand::undo() {
    auto* world = EditorContext::get().getWorld();
    if (!world) return;
    
    for (const auto& move : m_moves) {
        // Note: Would restore transform position on entity
        // world->getComponent<TransformComponent>(move.entity)->position = move.oldPosition;
    }
}

std::string MoveCommand::getName() const {
    if (m_moves.size() == 1) {
        return "Move Entity";
    }
    return "Move " + std::to_string(m_moves.size()) + " Entities";
}

bool MoveCommand::canMergeWith(const EditorCommand& other) const {
    if (m_mergeId == 0) return false;
    
    auto* otherMove = dynamic_cast<const MoveCommand*>(&other);
    if (!otherMove) return false;
    
    if (otherMove->m_mergeId != m_mergeId) return false;
    if (otherMove->m_moves.size() != m_moves.size()) return false;
    
    // Check same entities
    for (usize i = 0; i < m_moves.size(); ++i) {
        if (m_moves[i].entity != otherMove->m_moves[i].entity) {
            return false;
        }
    }
    
    return true;
}

bool MoveCommand::mergeWith(const EditorCommand& other) {
    auto* otherMove = dynamic_cast<const MoveCommand*>(&other);
    if (!otherMove) return false;
    
    // Keep our old positions, take their new positions
    for (usize i = 0; i < m_moves.size(); ++i) {
        m_moves[i].newPosition = otherMove->m_moves[i].newPosition;
    }
    
    return true;
}

// ============================================================================
// RotateCommand Implementation
// ============================================================================

RotateCommand::RotateCommand(ecs::Entity entity, const Quat& oldRotation, const Quat& newRotation) {
    m_rotations.push_back({entity, oldRotation, newRotation});
}

RotateCommand::RotateCommand(const std::vector<ecs::Entity>& entities,
                             const std::vector<Quat>& oldRotations,
                             const std::vector<Quat>& newRotations) {
    m_rotations.reserve(entities.size());
    for (usize i = 0; i < entities.size(); ++i) {
        m_rotations.push_back({
            entities[i],
            oldRotations[i],
            newRotations[i]
        });
    }
}

void RotateCommand::execute() {
    auto* world = EditorContext::get().getWorld();
    if (!world) return;
    
    for (const auto& rot : m_rotations) {
        // Note: Would set transform rotation on entity
    }
}

void RotateCommand::undo() {
    auto* world = EditorContext::get().getWorld();
    if (!world) return;
    
    for (const auto& rot : m_rotations) {
        // Note: Would restore transform rotation on entity
    }
}

std::string RotateCommand::getName() const {
    if (m_rotations.size() == 1) {
        return "Rotate Entity";
    }
    return "Rotate " + std::to_string(m_rotations.size()) + " Entities";
}

bool RotateCommand::canMergeWith(const EditorCommand& other) const {
    if (m_mergeId == 0) return false;
    
    auto* otherRot = dynamic_cast<const RotateCommand*>(&other);
    if (!otherRot) return false;
    
    if (otherRot->m_mergeId != m_mergeId) return false;
    if (otherRot->m_rotations.size() != m_rotations.size()) return false;
    
    for (usize i = 0; i < m_rotations.size(); ++i) {
        if (m_rotations[i].entity != otherRot->m_rotations[i].entity) {
            return false;
        }
    }
    
    return true;
}

bool RotateCommand::mergeWith(const EditorCommand& other) {
    auto* otherRot = dynamic_cast<const RotateCommand*>(&other);
    if (!otherRot) return false;
    
    for (usize i = 0; i < m_rotations.size(); ++i) {
        m_rotations[i].newRotation = otherRot->m_rotations[i].newRotation;
    }
    
    return true;
}

// ============================================================================
// ScaleCommand Implementation
// ============================================================================

ScaleCommand::ScaleCommand(ecs::Entity entity, const Vec3& oldScale, const Vec3& newScale) {
    m_scales.push_back({entity, oldScale, newScale});
}

ScaleCommand::ScaleCommand(const std::vector<ecs::Entity>& entities,
                           const std::vector<Vec3>& oldScales,
                           const std::vector<Vec3>& newScales) {
    m_scales.reserve(entities.size());
    for (usize i = 0; i < entities.size(); ++i) {
        m_scales.push_back({
            entities[i],
            oldScales[i],
            newScales[i]
        });
    }
}

void ScaleCommand::execute() {
    auto* world = EditorContext::get().getWorld();
    if (!world) return;
    
    for (const auto& scale : m_scales) {
        // Note: Would set transform scale on entity
    }
}

void ScaleCommand::undo() {
    auto* world = EditorContext::get().getWorld();
    if (!world) return;
    
    for (const auto& scale : m_scales) {
        // Note: Would restore transform scale on entity
    }
}

std::string ScaleCommand::getName() const {
    if (m_scales.size() == 1) {
        return "Scale Entity";
    }
    return "Scale " + std::to_string(m_scales.size()) + " Entities";
}

bool ScaleCommand::canMergeWith(const EditorCommand& other) const {
    if (m_mergeId == 0) return false;
    
    auto* otherScale = dynamic_cast<const ScaleCommand*>(&other);
    if (!otherScale) return false;
    
    if (otherScale->m_mergeId != m_mergeId) return false;
    if (otherScale->m_scales.size() != m_scales.size()) return false;
    
    for (usize i = 0; i < m_scales.size(); ++i) {
        if (m_scales[i].entity != otherScale->m_scales[i].entity) {
            return false;
        }
    }
    
    return true;
}

bool ScaleCommand::mergeWith(const EditorCommand& other) {
    auto* otherScale = dynamic_cast<const ScaleCommand*>(&other);
    if (!otherScale) return false;
    
    for (usize i = 0; i < m_scales.size(); ++i) {
        m_scales[i].newScale = otherScale->m_scales[i].newScale;
    }
    
    return true;
}

// ============================================================================
// TransformCommand Implementation
// ============================================================================

TransformCommand::TransformCommand(ecs::Entity entity, 
                                   const Transform& oldTransform, 
                                   const Transform& newTransform)
    : m_entity(entity)
    , m_oldTransform(oldTransform)
    , m_newTransform(newTransform)
{}

void TransformCommand::execute() {
    auto* world = EditorContext::get().getWorld();
    if (!world) return;
    
    // Note: Would set full transform on entity
}

void TransformCommand::undo() {
    auto* world = EditorContext::get().getWorld();
    if (!world) return;
    
    // Note: Would restore full transform on entity
}

std::string TransformCommand::getName() const {
    return "Transform Entity";
}

// ============================================================================
// CreateEntityCommand Implementation
// ============================================================================

CreateEntityCommand::CreateEntityCommand(const std::string& name)
    : m_name(name)
{}

void CreateEntityCommand::execute() {
    auto* world = EditorContext::get().getWorld();
    if (!world) return;
    
    if (!m_executed) {
        m_entity = world->createEntity();
        m_executed = true;
    } else {
        // Re-create with same ID if possible
        m_entity = world->createEntity();
    }
    
    // Note: Would add name component with m_name
}

void CreateEntityCommand::undo() {
    auto* world = EditorContext::get().getWorld();
    if (!world) return;
    
    world->destroyEntity(m_entity);
}

// ============================================================================
// DeleteEntitiesCommand Implementation
// ============================================================================

DeleteEntitiesCommand::DeleteEntitiesCommand(ecs::Entity entity) {
    m_entities.push_back({entity, ecs::Entity(), "", Transform(), {}});
}

DeleteEntitiesCommand::DeleteEntitiesCommand(const std::vector<ecs::Entity>& entities) {
    m_entities.reserve(entities.size());
    for (auto entity : entities) {
        m_entities.push_back({entity, ecs::Entity(), "", Transform(), {}});
    }
}

void DeleteEntitiesCommand::execute() {
    auto* world = EditorContext::get().getWorld();
    if (!world) return;
    
    for (auto& data : m_entities) {
        // Note: Would serialize entity data before deletion
        // data.serializedComponents = serializeEntity(data.entity);
        // data.transform = getTransform(data.entity);
        // data.parent = getParent(data.entity);
        // data.name = getName(data.entity);
        
        world->destroyEntity(data.entity);
    }
}

void DeleteEntitiesCommand::undo() {
    auto* world = EditorContext::get().getWorld();
    if (!world) return;
    
    for (auto& data : m_entities) {
        // Recreate entity
        data.entity = world->createEntity();
        
        // Note: Would deserialize entity data
        // deserializeEntity(data.entity, data.serializedComponents);
        // setTransform(data.entity, data.transform);
        // setParent(data.entity, data.parent);
        // setName(data.entity, data.name);
    }
}

std::string DeleteEntitiesCommand::getName() const {
    if (m_entities.size() == 1) {
        return "Delete Entity";
    }
    return "Delete " + std::to_string(m_entities.size()) + " Entities";
}

// ============================================================================
// DuplicateEntitiesCommand Implementation
// ============================================================================

DuplicateEntitiesCommand::DuplicateEntitiesCommand(ecs::Entity entity) {
    m_originals.push_back(entity);
}

DuplicateEntitiesCommand::DuplicateEntitiesCommand(const std::vector<ecs::Entity>& entities)
    : m_originals(entities)
{}

void DuplicateEntitiesCommand::execute() {
    auto* world = EditorContext::get().getWorld();
    if (!world) return;
    
    m_duplicates.clear();
    m_duplicates.reserve(m_originals.size());
    
    for (auto original : m_originals) {
        // Note: Would clone entity with all components
        auto duplicate = world->createEntity();
        m_duplicates.push_back(duplicate);
    }
    
    // Select duplicates
    EditorContext::get().clearSelection();
    for (auto entity : m_duplicates) {
        EditorContext::get().addToSelection(entity);
    }
}

void DuplicateEntitiesCommand::undo() {
    auto* world = EditorContext::get().getWorld();
    if (!world) return;
    
    for (auto duplicate : m_duplicates) {
        world->destroyEntity(duplicate);
    }
    m_duplicates.clear();
    
    // Restore selection to originals
    EditorContext::get().clearSelection();
    for (auto entity : m_originals) {
        EditorContext::get().addToSelection(entity);
    }
}

std::string DuplicateEntitiesCommand::getName() const {
    if (m_originals.size() == 1) {
        return "Duplicate Entity";
    }
    return "Duplicate " + std::to_string(m_originals.size()) + " Entities";
}

// ============================================================================
// RenameEntityCommand Implementation
// ============================================================================

RenameEntityCommand::RenameEntityCommand(ecs::Entity entity, 
                                         const std::string& oldName,
                                         const std::string& newName)
    : m_entity(entity)
    , m_oldName(oldName)
    , m_newName(newName)
{}

void RenameEntityCommand::execute() {
    // Note: Would set name component
}

void RenameEntityCommand::undo() {
    // Note: Would restore name component
}

// ============================================================================
// ReparentEntityCommand Implementation
// ============================================================================

ReparentEntityCommand::ReparentEntityCommand(ecs::Entity entity,
                                             ecs::Entity oldParent,
                                             ecs::Entity newParent)
    : m_entity(entity)
    , m_oldParent(oldParent)
    , m_newParent(newParent)
{}

void ReparentEntityCommand::execute() {
    // Note: Would change parent relationship
}

void ReparentEntityCommand::undo() {
    // Note: Would restore parent relationship
}

// ============================================================================
// SelectionCommand Implementation
// ============================================================================

SelectionCommand::SelectionCommand(const Selection& oldSelection, const Selection& newSelection)
    : m_oldSelection(oldSelection)
    , m_newSelection(newSelection)
{}

void SelectionCommand::execute() {
    auto& ctx = EditorContext::get();
    ctx.clearSelection();
    for (const auto& info : m_newSelection.entities) {
        ctx.addToSelection(info.entity);
    }
}

void SelectionCommand::undo() {
    auto& ctx = EditorContext::get();
    ctx.clearSelection();
    for (const auto& info : m_oldSelection.entities) {
        ctx.addToSelection(info.entity);
    }
}

// ============================================================================
// CompositeCommand Implementation
// ============================================================================

CompositeCommand::CompositeCommand(const std::string& name)
    : m_name(name)
{}

void CompositeCommand::addCommand(std::unique_ptr<EditorCommand> command) {
    m_commands.push_back(std::move(command));
}

void CompositeCommand::execute() {
    for (auto& cmd : m_commands) {
        cmd->execute();
    }
}

void CompositeCommand::undo() {
    // Undo in reverse order
    for (auto it = m_commands.rbegin(); it != m_commands.rend(); ++it) {
        (*it)->undo();
    }
}

// ============================================================================
// LambdaCommand Implementation
// ============================================================================

LambdaCommand::LambdaCommand(const std::string& name,
                             std::function<void()> executeFunc,
                             std::function<void()> undoFunc)
    : m_name(name)
    , m_executeFunc(std::move(executeFunc))
    , m_undoFunc(std::move(undoFunc))
{}

} // namespace nova::editor
