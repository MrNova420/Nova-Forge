/**
 * @file editor_command.hpp
 * @brief NovaCore Editor™ - Command Pattern for Undo/Redo System
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 * 
 * Implements the Command pattern for the undo/redo system. All editor
 * operations that modify state should be implemented as commands to
 * support full undo/redo functionality.
 */

#pragma once

#include "editor_types.hpp"
#include <nova/core/ecs/entity.hpp>
#include <nova/core/math/transform.hpp>

#include <string>
#include <memory>
#include <vector>
#include <functional>

namespace nova::editor {

// ============================================================================
// Base Command Class
// ============================================================================

/**
 * @brief Base class for all editor commands
 * 
 * Commands encapsulate operations that can be undone and redone.
 * Each command stores enough state to reverse its operation.
 * 
 * Usage:
 * @code
 *     class MyCommand : public EditorCommand {
 *     public:
 *         void execute() override { ... }
 *         void undo() override { ... }
 *     };
 *     
 *     EditorContext::get().executeCommand<MyCommand>(arg1, arg2);
 * @endcode
 */
class EditorCommand {
public:
    virtual ~EditorCommand() = default;
    
    /**
     * @brief Execute the command
     * 
     * Called when the command is first executed and when it's redone.
     */
    virtual void execute() = 0;
    
    /**
     * @brief Undo the command
     * 
     * Called when the user undoes this command.
     * Must restore state to exactly what it was before execute().
     */
    virtual void undo() = 0;
    
    /**
     * @brief Get command name for display
     */
    virtual std::string getName() const = 0;
    
    /**
     * @brief Get detailed description
     */
    virtual std::string getDescription() const { return getName(); }
    
    /**
     * @brief Check if this command can be merged with another
     * 
     * Some commands (like continuous dragging) can be merged into one
     * undo entry for better UX.
     */
    virtual bool canMergeWith(const EditorCommand& other) const { return false; }
    
    /**
     * @brief Merge another command into this one
     * @return true if merge succeeded
     */
    virtual bool mergeWith(const EditorCommand& other) { return false; }
    
    /**
     * @brief Get merge ID for grouping mergeable commands
     * 
     * Commands with the same non-zero merge ID can potentially be merged.
     */
    virtual u64 getMergeId() const { return 0; }
    
    /**
     * @brief Check if command is valid and can be executed
     */
    virtual bool isValid() const { return true; }
    
    /**
     * @brief Get the command info
     */
    CommandInfo getInfo() const {
        CommandInfo info;
        info.name = getName();
        info.description = getDescription();
        info.timestamp = std::chrono::system_clock::now();
        info.isMergeable = getMergeId() != 0;
        info.mergeId = getMergeId();
        return info;
    }
};

// ============================================================================
// Transform Commands
// ============================================================================

/**
 * @brief Command to move entities
 */
class MoveCommand : public EditorCommand {
public:
    MoveCommand(ecs::Entity entity, const Vec3& oldPosition, const Vec3& newPosition);
    MoveCommand(const std::vector<ecs::Entity>& entities, 
                const std::vector<Vec3>& oldPositions,
                const std::vector<Vec3>& newPositions);
    
    void execute() override;
    void undo() override;
    std::string getName() const override;
    
    bool canMergeWith(const EditorCommand& other) const override;
    bool mergeWith(const EditorCommand& other) override;
    u64 getMergeId() const override { return m_mergeId; }
    
    void setMergeId(u64 id) { m_mergeId = id; }
    
private:
    struct EntityMove {
        ecs::Entity entity;
        Vec3 oldPosition;
        Vec3 newPosition;
    };
    std::vector<EntityMove> m_moves;
    u64 m_mergeId = 0;
};

/**
 * @brief Command to rotate entities
 */
class RotateCommand : public EditorCommand {
public:
    RotateCommand(ecs::Entity entity, const Quat& oldRotation, const Quat& newRotation);
    RotateCommand(const std::vector<ecs::Entity>& entities,
                  const std::vector<Quat>& oldRotations,
                  const std::vector<Quat>& newRotations);
    
    void execute() override;
    void undo() override;
    std::string getName() const override;
    
    bool canMergeWith(const EditorCommand& other) const override;
    bool mergeWith(const EditorCommand& other) override;
    u64 getMergeId() const override { return m_mergeId; }
    
    void setMergeId(u64 id) { m_mergeId = id; }
    
private:
    struct EntityRotate {
        ecs::Entity entity;
        Quat oldRotation;
        Quat newRotation;
    };
    std::vector<EntityRotate> m_rotations;
    u64 m_mergeId = 0;
};

/**
 * @brief Command to scale entities
 */
class ScaleCommand : public EditorCommand {
public:
    ScaleCommand(ecs::Entity entity, const Vec3& oldScale, const Vec3& newScale);
    ScaleCommand(const std::vector<ecs::Entity>& entities,
                 const std::vector<Vec3>& oldScales,
                 const std::vector<Vec3>& newScales);
    
    void execute() override;
    void undo() override;
    std::string getName() const override;
    
    bool canMergeWith(const EditorCommand& other) const override;
    bool mergeWith(const EditorCommand& other) override;
    u64 getMergeId() const override { return m_mergeId; }
    
    void setMergeId(u64 id) { m_mergeId = id; }
    
private:
    struct EntityScale {
        ecs::Entity entity;
        Vec3 oldScale;
        Vec3 newScale;
    };
    std::vector<EntityScale> m_scales;
    u64 m_mergeId = 0;
};

/**
 * @brief Command for full transform change
 */
class TransformCommand : public EditorCommand {
public:
    TransformCommand(ecs::Entity entity, const Transform& oldTransform, const Transform& newTransform);
    
    void execute() override;
    void undo() override;
    std::string getName() const override;
    
private:
    ecs::Entity m_entity;
    Transform m_oldTransform;
    Transform m_newTransform;
};

// ============================================================================
// Entity Commands
// ============================================================================

/**
 * @brief Command to create entity
 */
class CreateEntityCommand : public EditorCommand {
public:
    explicit CreateEntityCommand(const std::string& name = "New Entity");
    
    void execute() override;
    void undo() override;
    std::string getName() const override { return "Create Entity"; }
    
    ecs::Entity getCreatedEntity() const { return m_entity; }
    
private:
    std::string m_name;
    ecs::Entity m_entity;
    bool m_executed = false;
};

/**
 * @brief Command to delete entities
 */
class DeleteEntitiesCommand : public EditorCommand {
public:
    explicit DeleteEntitiesCommand(ecs::Entity entity);
    explicit DeleteEntitiesCommand(const std::vector<ecs::Entity>& entities);
    
    void execute() override;
    void undo() override;
    std::string getName() const override;
    
private:
    struct EntityData {
        ecs::Entity entity;
        ecs::Entity parent;
        std::string name;
        Transform transform;
        std::vector<u8> serializedComponents;
    };
    std::vector<EntityData> m_entities;
};

/**
 * @brief Command to duplicate entities
 */
class DuplicateEntitiesCommand : public EditorCommand {
public:
    explicit DuplicateEntitiesCommand(ecs::Entity entity);
    explicit DuplicateEntitiesCommand(const std::vector<ecs::Entity>& entities);
    
    void execute() override;
    void undo() override;
    std::string getName() const override;
    
    std::vector<ecs::Entity> getDuplicatedEntities() const { return m_duplicates; }
    
private:
    std::vector<ecs::Entity> m_originals;
    std::vector<ecs::Entity> m_duplicates;
};

/**
 * @brief Command to rename entity
 */
class RenameEntityCommand : public EditorCommand {
public:
    RenameEntityCommand(ecs::Entity entity, const std::string& oldName, const std::string& newName);
    
    void execute() override;
    void undo() override;
    std::string getName() const override { return "Rename Entity"; }
    
private:
    ecs::Entity m_entity;
    std::string m_oldName;
    std::string m_newName;
};

/**
 * @brief Command to reparent entity
 */
class ReparentEntityCommand : public EditorCommand {
public:
    ReparentEntityCommand(ecs::Entity entity, ecs::Entity oldParent, ecs::Entity newParent);
    
    void execute() override;
    void undo() override;
    std::string getName() const override { return "Reparent Entity"; }
    
private:
    ecs::Entity m_entity;
    ecs::Entity m_oldParent;
    ecs::Entity m_newParent;
    i32 m_oldSiblingIndex = -1;
};

// ============================================================================
// Component Commands
// ============================================================================

/**
 * @brief Command to add component to entity
 */
template<typename T>
class AddComponentCommand : public EditorCommand {
public:
    AddComponentCommand(ecs::Entity entity, const T& component = T{});
    
    void execute() override;
    void undo() override;
    std::string getName() const override { return "Add Component"; }
    
private:
    ecs::Entity m_entity;
    T m_component;
};

/**
 * @brief Command to remove component from entity
 */
template<typename T>
class RemoveComponentCommand : public EditorCommand {
public:
    explicit RemoveComponentCommand(ecs::Entity entity);
    
    void execute() override;
    void undo() override;
    std::string getName() const override { return "Remove Component"; }
    
private:
    ecs::Entity m_entity;
    T m_savedComponent;
};

/**
 * @brief Command to modify component property
 */
template<typename T, typename ValueType>
class ModifyComponentCommand : public EditorCommand {
public:
    using GetterFunc = std::function<ValueType(const T&)>;
    using SetterFunc = std::function<void(T&, const ValueType&)>;
    
    ModifyComponentCommand(ecs::Entity entity,
                           const std::string& propertyName,
                           const ValueType& oldValue,
                           const ValueType& newValue,
                           SetterFunc setter);
    
    void execute() override;
    void undo() override;
    std::string getName() const override { return "Modify " + m_propertyName; }
    
    bool canMergeWith(const EditorCommand& other) const override;
    bool mergeWith(const EditorCommand& other) override;
    u64 getMergeId() const override { return m_mergeId; }
    
    void setMergeId(u64 id) { m_mergeId = id; }
    
private:
    ecs::Entity m_entity;
    std::string m_propertyName;
    ValueType m_oldValue;
    ValueType m_newValue;
    SetterFunc m_setter;
    u64 m_mergeId = 0;
};

// ============================================================================
// Selection Commands
// ============================================================================

/**
 * @brief Command to change selection
 */
class SelectionCommand : public EditorCommand {
public:
    SelectionCommand(const Selection& oldSelection, const Selection& newSelection);
    
    void execute() override;
    void undo() override;
    std::string getName() const override { return "Change Selection"; }
    
private:
    Selection m_oldSelection;
    Selection m_newSelection;
};

// ============================================================================
// Composite Commands
// ============================================================================

/**
 * @brief Command that groups multiple commands
 */
class CompositeCommand : public EditorCommand {
public:
    explicit CompositeCommand(const std::string& name);
    
    void addCommand(std::unique_ptr<EditorCommand> command);
    
    void execute() override;
    void undo() override;
    std::string getName() const override { return m_name; }
    
    bool isEmpty() const { return m_commands.empty(); }
    usize getCommandCount() const { return m_commands.size(); }
    
private:
    std::string m_name;
    std::vector<std::unique_ptr<EditorCommand>> m_commands;
};

// ============================================================================
// Lambda Command (for quick prototyping)
// ============================================================================

/**
 * @brief Command defined by lambda functions
 */
class LambdaCommand : public EditorCommand {
public:
    LambdaCommand(const std::string& name,
                  std::function<void()> executeFunc,
                  std::function<void()> undoFunc);
    
    void execute() override { m_executeFunc(); }
    void undo() override { m_undoFunc(); }
    std::string getName() const override { return m_name; }
    
private:
    std::string m_name;
    std::function<void()> m_executeFunc;
    std::function<void()> m_undoFunc;
};

// ============================================================================
// Template Implementations
// ============================================================================

template<typename T>
AddComponentCommand<T>::AddComponentCommand(ecs::Entity entity, const T& component)
    : m_entity(entity)
    , m_component(component)
{}

template<typename T>
void AddComponentCommand<T>::execute() {
    // Note: Implementation would access EditorContext::get().getWorld()
    // and add the component to the entity
}

template<typename T>
void AddComponentCommand<T>::undo() {
    // Note: Implementation would remove the component from the entity
}

template<typename T>
RemoveComponentCommand<T>::RemoveComponentCommand(ecs::Entity entity)
    : m_entity(entity)
{}

template<typename T>
void RemoveComponentCommand<T>::execute() {
    // Save component data before removing
    // Note: Implementation would access the world and save/remove component
}

template<typename T>
void RemoveComponentCommand<T>::undo() {
    // Restore the saved component
}

template<typename T, typename ValueType>
ModifyComponentCommand<T, ValueType>::ModifyComponentCommand(
    ecs::Entity entity,
    const std::string& propertyName,
    const ValueType& oldValue,
    const ValueType& newValue,
    SetterFunc setter)
    : m_entity(entity)
    , m_propertyName(propertyName)
    , m_oldValue(oldValue)
    , m_newValue(newValue)
    , m_setter(setter)
{}

template<typename T, typename ValueType>
void ModifyComponentCommand<T, ValueType>::execute() {
    // Get component and apply new value
    // Note: Implementation would use m_setter(component, m_newValue)
}

template<typename T, typename ValueType>
void ModifyComponentCommand<T, ValueType>::undo() {
    // Get component and apply old value
    // Note: Implementation would use m_setter(component, m_oldValue)
}

template<typename T, typename ValueType>
bool ModifyComponentCommand<T, ValueType>::canMergeWith(const EditorCommand& other) const {
    if (m_mergeId == 0) return false;
    
    auto* otherCmd = dynamic_cast<const ModifyComponentCommand<T, ValueType>*>(&other);
    if (!otherCmd) return false;
    
    return m_mergeId == otherCmd->m_mergeId &&
           m_entity == otherCmd->m_entity &&
           m_propertyName == otherCmd->m_propertyName;
}

template<typename T, typename ValueType>
bool ModifyComponentCommand<T, ValueType>::mergeWith(const EditorCommand& other) {
    auto* otherCmd = dynamic_cast<const ModifyComponentCommand<T, ValueType>*>(&other);
    if (!otherCmd) return false;
    
    // Keep our old value, take their new value
    m_newValue = otherCmd->m_newValue;
    return true;
}

} // namespace nova::editor
