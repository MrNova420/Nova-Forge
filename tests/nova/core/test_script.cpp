/**
 * @file test_script.cpp
 * @brief Unit tests for NovaCore Script System
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <nova/core/script/script.hpp>

using namespace nova;
using namespace nova::script;
using namespace nova::math;
using Catch::Approx;

// ============================================================================
// Script Types Tests
// ============================================================================

TEST_CASE("Script Types - ScriptType Enum", "[script][types]") {
    SECTION("All script types are defined") {
        REQUIRE(static_cast<u8>(ScriptType::Void) == 0);
        REQUIRE(static_cast<u8>(ScriptType::Bool) == 1);
        REQUIRE(static_cast<u8>(ScriptType::Int) == 2);
        REQUIRE(static_cast<u8>(ScriptType::Float) == 3);
        REQUIRE(static_cast<u8>(ScriptType::String) == 4);
        REQUIRE(static_cast<u8>(ScriptType::Vec2) == 5);
        REQUIRE(static_cast<u8>(ScriptType::Vec3) == 6);
        REQUIRE(static_cast<u8>(ScriptType::Vec4) == 7);
        REQUIRE(static_cast<u8>(ScriptType::Quat) == 8);
        REQUIRE(static_cast<u8>(ScriptType::Color) == 9);
        REQUIRE(static_cast<u8>(ScriptType::Entity) == 10);
        REQUIRE(static_cast<u8>(ScriptType::Object) == 11);
        REQUIRE(static_cast<u8>(ScriptType::Array) == 12);
        REQUIRE(static_cast<u8>(ScriptType::Map) == 13);
        REQUIRE(static_cast<u8>(ScriptType::Function) == 14);
        REQUIRE(static_cast<u8>(ScriptType::Any) == 15);
    }
}

TEST_CASE("Script Types - ScriptValue Construction", "[script][types]") {
    SECTION("Default construction") {
        ScriptValue val;
        REQUIRE(val.type == ScriptType::Void);
        REQUIRE(val.isVoid() == true);
    }
    
    SECTION("Bool value") {
        ScriptValue val(true);
        REQUIRE(val.type == ScriptType::Bool);
        REQUIRE(val.isBool() == true);
        REQUIRE(val.asBool() == true);
    }
    
    SECTION("Int value") {
        ScriptValue val(42);
        REQUIRE(val.type == ScriptType::Int);
        REQUIRE(val.isInt() == true);
        REQUIRE(val.asInt() == 42);
    }
    
    SECTION("Float value") {
        ScriptValue val(3.14f);
        REQUIRE(val.type == ScriptType::Float);
        REQUIRE(val.isFloat() == true);
        REQUIRE(val.asFloat() == Approx(3.14));
    }
    
    SECTION("String value") {
        ScriptValue val("Hello World");
        REQUIRE(val.type == ScriptType::String);
        REQUIRE(val.isString() == true);
        REQUIRE(val.asString() == "Hello World");
    }
    
    SECTION("Vec3 value") {
        ScriptValue val(Vec3(1.0f, 2.0f, 3.0f));
        REQUIRE(val.type == ScriptType::Vec3);
        REQUIRE(val.isVec3() == true);
        Vec3 v = val.asVec3();
        REQUIRE(v.x == Approx(1.0f));
        REQUIRE(v.y == Approx(2.0f));
        REQUIRE(v.z == Approx(3.0f));
    }
}

TEST_CASE("Script Types - ScriptValue Type Checking", "[script][types]") {
    SECTION("isNumber combines int and float") {
        ScriptValue intVal(42);
        ScriptValue floatVal(3.14f);
        ScriptValue strVal("test");
        
        REQUIRE(intVal.isNumber() == true);
        REQUIRE(floatVal.isNumber() == true);
        REQUIRE(strVal.isNumber() == false);
    }
}

TEST_CASE("Script Types - ScriptConfig", "[script][types]") {
    SECTION("Configuration constants") {
        REQUIRE(ScriptConfig::MAX_FUNCTION_PARAMS == 16);
        REQUIRE(ScriptConfig::MAX_CALL_STACK_DEPTH == 256);
        REQUIRE(ScriptConfig::MAX_LOCAL_VARIABLES == 128);
        REQUIRE(ScriptConfig::DEFAULT_STACK_SIZE == 1024 * 64);
        REQUIRE(ScriptConfig::EXECUTION_TIME_LIMIT == Approx(5.0f));
    }
}

TEST_CASE("Script Types - NodeType Enum", "[script][types]") {
    SECTION("Control flow node types") {
        REQUIRE(static_cast<u8>(NodeType::Entry) == 0);
        REQUIRE(static_cast<u8>(NodeType::Return) == 1);
        REQUIRE(static_cast<u8>(NodeType::Branch) == 2);
        REQUIRE(static_cast<u8>(NodeType::Switch) == 3);
        REQUIRE(static_cast<u8>(NodeType::Sequence) == 4);
        REQUIRE(static_cast<u8>(NodeType::ForLoop) == 5);
        REQUIRE(static_cast<u8>(NodeType::WhileLoop) == 6);
        REQUIRE(static_cast<u8>(NodeType::ForEach) == 7);
    }
}

TEST_CASE("Script Types - ScriptParam", "[script][types]") {
    SECTION("Default parameter") {
        ScriptParam param;
        REQUIRE(param.type == ScriptType::Any);
        REQUIRE(param.hasDefault == false);
        REQUIRE(param.isOptional == false);
        REQUIRE(param.isRef == false);
    }
    
    SECTION("Configured parameter") {
        ScriptParam param;
        param.name = "value";
        param.type = ScriptType::Float;
        param.hasDefault = true;
        param.defaultValue = ScriptValue(1.0f);
        
        REQUIRE(param.name == "value");
        REQUIRE(param.type == ScriptType::Float);
        REQUIRE(param.hasDefault == true);
        REQUIRE(param.defaultValue.asFloat() == Approx(1.0));
    }
}
