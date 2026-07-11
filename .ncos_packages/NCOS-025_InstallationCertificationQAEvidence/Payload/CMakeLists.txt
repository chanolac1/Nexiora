cmake_minimum_required(VERSION 3.25)

project(Nexiora
    VERSION 0.1.0
    DESCRIPTION "Nexiora Autonomous Laboratory Foundation"
    LANGUAGES C
)

if(MSVC)
    set(CMAKE_C_STANDARD 17)
else()
    set(CMAKE_C_STANDARD 23)
endif()

set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(NEXIORA_BUILD_TESTS "Build Nexiora tests" ON)
option(NEXIORA_ENABLE_BENCHMARKS "Build Nexiora benchmarks" ON)

if(MSVC)
    add_compile_options(/W4 /permissive- /utf-8 /std:clatest)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_library(NexioraNCP STATIC
    Source/NCP/Common/NxResult.c
    Source/NCP/Runtime/NxRuntime.c
    Source/NCP/Memory/NxMemory.c
    Source/NCP/Logging/NxLog.c
    Source/NCP/Hardware/NxHardware.c
    Source/NCP/String/NxString.c
    Source/NCP/Containers/NxVector.c
    Source/NCP/Benchmark/NxBenchmark.c
    Source/NCP/Evidence/NxEvidence.c
    Source/NCP/Research/NxJournal.c
    Source/NCP/Research/NxExperimentRunner.c
    Source/NCP/Research/NxResearchEvidence.c
    Source/NCP/Research/NxPromotion.c
    Source/Research/NxExperiment.c
    Source/Research/NxResearchKernel.c
    Source/Research/NxManifest.c
    Source/Research/NxRegistry.c
    Source/Research/NxAutonomousResearchPipeline.c
    Source/Research/NxDiscoveryEngine.c
    Source/Research/NxResearchGraph.c
    Source/Research/NxResearchGraphExporter.c
    Source/Research/NxResearchGraphQuery.c
    Source/Research/NxResearchScheduler.c
    Source/Research/NxAutonomousExecution.c
    Source/Research/NxResearchDashboard.c
    Source/Research/NxResearchQueue.c
    Source/Research/NxResearchPolicy.c
    Source/Research/NxResearchSession.c
    Source/Research/NxAutonomousResearchLoop.c
    Source/Research/NxKnowledgeBase.c
    Source/Research/NxKnowledgeQuery.c
    Source/Research/NxHypothesisEngine.c
    Source/Research/NxReasoningEngine.c
    Source/Research/NxEvidenceCorrelation.c
    Source/Research/NxConflictDetector.c
    Source/Research/NxProgressEngine.c
    Source/Conversation/NxConversation.c
    Source/Research/NxPersistentMemory.c
    Source/Research/NxKnowledgeAcquisition.c
    Source/NCOS/NxCompilerEngine.c
    Source/NCOS/NxBuildLogAnalyzer.c
    
    Source/NCOS/NxAutoFixProposalEngine.c
    Source/Research/NxKnowledgeStore.c
    Source/Research/NxLearningCore.c
    Source/Research/NxTopicQuestion.c
    Source/Research/NxTopicInvestigation.c
)

target_include_directories(NexioraNCP PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
)

target_compile_definitions(NexioraNCP PUBLIC
    NEXIORA_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    NEXIORA_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    NEXIORA_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(NexioraNCP PRIVATE -mxsave)
endif()

add_executable(nexiora Source/NxBootstrap.c)
target_link_libraries(nexiora PRIVATE NexioraNCP)

if(NEXIORA_ENABLE_BENCHMARKS)
    add_executable(nexiora_bench Tests/NxBenchmarkMain.c)
    target_link_libraries(nexiora_bench PRIVATE NexioraNCP)

    add_executable(NxResearchSchedulerBench Tests/Benchmark/NxResearchSchedulerBench.c)
    target_link_libraries(NxResearchSchedulerBench PRIVATE NexioraNCP)

    add_executable(NxResearchGraphBenchmark Tests/Benchmark/NxResearchGraphBenchmark.c)
    target_link_libraries(NxResearchGraphBenchmark PRIVATE NexioraNCP)

    add_executable(NxResearchGraphExporterBenchmark Tests/Benchmark/NxResearchGraphExporterBenchmark.c)
    target_link_libraries(NxResearchGraphExporterBenchmark PRIVATE NexioraNCP)
endif()

if(NEXIORA_BUILD_TESTS)
    enable_testing()

    add_executable(nexiora_tests Tests/NxCoreTests.c)
    target_link_libraries(nexiora_tests PRIVATE NexioraNCP)
    add_test(NAME NexioraCoreTests COMMAND nexiora_tests)

    add_executable(NxVectorTests Tests/Unit/NxVectorTests.c)
    target_link_libraries(NxVectorTests PRIVATE NexioraNCP)
    add_test(NAME NxVectorTests COMMAND NxVectorTests)

    add_executable(NxResearchTests Tests/Unit/NxResearchTests.c)
    target_link_libraries(NxResearchTests PRIVATE NexioraNCP)
    add_test(NAME NxResearchTests COMMAND NxResearchTests)

    add_executable(NxJournalTests Tests/Unit/NxJournalTests.c)
    target_link_libraries(NxJournalTests PRIVATE NexioraNCP)
    add_test(NAME NxJournalTests COMMAND NxJournalTests)

    add_executable(NxExperimentRunnerTests Tests/Unit/NxExperimentRunnerTests.c)
    target_link_libraries(NxExperimentRunnerTests PRIVATE NexioraNCP)
    add_test(NAME NxExperimentRunnerTests COMMAND NxExperimentRunnerTests)

    add_executable(NxResearchEvidenceTests Tests/Unit/NxResearchEvidenceTests.c)
    target_link_libraries(NxResearchEvidenceTests PRIVATE NexioraNCP)
    add_test(NAME NxResearchEvidenceTests COMMAND NxResearchEvidenceTests)

    add_executable(NxPromotionTests Tests/Unit/NxPromotionTests.c)
    target_link_libraries(NxPromotionTests PRIVATE NexioraNCP)
    add_test(NAME NxPromotionTests COMMAND NxPromotionTests)

    add_executable(NxAutonomousResearchPipelineTests Tests/Unit/NxAutonomousResearchPipelineTests.c)
    target_link_libraries(NxAutonomousResearchPipelineTests PRIVATE NexioraNCP)
    add_test(NAME NxAutonomousResearchPipelineTests COMMAND NxAutonomousResearchPipelineTests)

    add_executable(NxDiscoveryEngineTests Tests/Unit/NxDiscoveryEngineTests.c)
    target_link_libraries(NxDiscoveryEngineTests PRIVATE NexioraNCP)
    add_test(NAME NxDiscoveryEngineTests COMMAND NxDiscoveryEngineTests)

    add_executable(NxResearchGraphTests Tests/Unit/NxResearchGraphTests.c)
    target_link_libraries(NxResearchGraphTests PRIVATE NexioraNCP)
    add_test(NAME NxResearchGraphTests COMMAND NxResearchGraphTests)

    add_executable(NxResearchGraphExporterTests Tests/Unit/NxResearchGraphExporterTests.c)
    target_link_libraries(NxResearchGraphExporterTests PRIVATE NexioraNCP)
    add_test(NAME NxResearchGraphExporterTests COMMAND NxResearchGraphExporterTests)

    add_executable(NxResearchGraphQueryTests Tests/Unit/NxResearchGraphQueryTests.c)
    target_link_libraries(NxResearchGraphQueryTests PRIVATE NexioraNCP)
    add_test(NAME NxResearchGraphQueryTests COMMAND NxResearchGraphQueryTests)

    add_executable(NxResearchSchedulerTests Tests/Unit/NxResearchSchedulerTests.c)
    target_link_libraries(NxResearchSchedulerTests PRIVATE NexioraNCP)
    add_test(NAME NxResearchSchedulerTests COMMAND NxResearchSchedulerTests)

    add_executable(NxResearchQueueTests Tests/Unit/NxResearchQueueTests.c)
    target_link_libraries(NxResearchQueueTests PRIVATE NexioraNCP)
    add_test(NAME NxResearchQueueTests COMMAND NxResearchQueueTests)

    add_executable(NxResearchPolicyTests Tests/Unit/NxResearchPolicyTests.c)
    target_link_libraries(NxResearchPolicyTests PRIVATE NexioraNCP)
    add_test(NAME NxResearchPolicyTests COMMAND NxResearchPolicyTests)

    add_executable(NxResearchSessionTests Tests/Unit/NxResearchSessionTests.c)
    target_link_libraries(NxResearchSessionTests PRIVATE NexioraNCP)
    add_test(NAME NxResearchSessionTests COMMAND NxResearchSessionTests)

    add_executable(NxAutonomousResearchLoopTests Tests/Unit/NxAutonomousResearchLoopTests.c)
    target_link_libraries(NxAutonomousResearchLoopTests PRIVATE NexioraNCP)
    add_test(NAME NxAutonomousResearchLoopTests COMMAND NxAutonomousResearchLoopTests)

    add_executable(NxKnowledgeBaseTests Tests/Unit/NxKnowledgeBaseTests.c)
    target_link_libraries(NxKnowledgeBaseTests PRIVATE NexioraNCP)
    add_test(NAME NxKnowledgeBaseTests COMMAND NxKnowledgeBaseTests)

    add_executable(NxKnowledgeQueryTests Tests/Unit/NxKnowledgeQueryTests.c)
    target_link_libraries(NxKnowledgeQueryTests PRIVATE NexioraNCP)
    add_test(NAME NxKnowledgeQueryTests COMMAND NxKnowledgeQueryTests)

    add_executable(NxHypothesisEngineTests Tests/Unit/NxHypothesisEngineTests.c)
    target_link_libraries(NxHypothesisEngineTests PRIVATE NexioraNCP)
    add_test(NAME NxHypothesisEngineTests COMMAND NxHypothesisEngineTests)

    add_executable(NxReasoningEngineTests Tests/Unit/NxReasoningEngineTests.c)
    target_link_libraries(NxReasoningEngineTests PRIVATE NexioraNCP)
    add_test(NAME NxReasoningEngineTests COMMAND NxReasoningEngineTests)

    add_executable(NxEvidenceCorrelationTests Tests/Unit/NxEvidenceCorrelationTests.c)
    target_link_libraries(NxEvidenceCorrelationTests PRIVATE NexioraNCP)
    add_test(NAME NxEvidenceCorrelationTests COMMAND NxEvidenceCorrelationTests)

    add_executable(NxConflictDetectorTests Tests/Unit/NxConflictDetectorTests.c)
    target_link_libraries(NxConflictDetectorTests PRIVATE NexioraNCP)
    add_test(NAME NxConflictDetectorTests COMMAND NxConflictDetectorTests)


    add_executable(NxPersistentMemoryTests Tests/Unit/NxPersistentMemoryTests.c)
    target_link_libraries(NxPersistentMemoryTests PRIVATE NexioraNCP)
    add_test(NAME NxPersistentMemoryTests COMMAND NxPersistentMemoryTests)

    set_tests_properties(
        NexioraCoreTests
        NxVectorTests
        NxResearchTests
        NxJournalTests
        NxExperimentRunnerTests
        NxResearchEvidenceTests
        NxPromotionTests
        NxAutonomousResearchPipelineTests
        NxDiscoveryEngineTests
        NxResearchGraphTests
        NxResearchGraphExporterTests
        NxResearchGraphQueryTests
        NxResearchSchedulerTests
        NxResearchQueueTests
        NxResearchPolicyTests
        NxResearchSessionTests
        NxAutonomousResearchLoopTests
        NxKnowledgeBaseTests
        NxKnowledgeQueryTests
        NxHypothesisEngineTests
        NxReasoningEngineTests
        NxEvidenceCorrelationTests
        NxConflictDetectorTests
        NxPersistentMemoryTests
        PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()


if(NEXIORA_BUILD_TESTS)
    add_executable(NxConversationTests
        Tests/Unit/NxConversationTests.c
    )
    target_link_libraries(NxConversationTests PRIVATE NexioraNCP)
    add_test(NAME NxConversationTests COMMAND NxConversationTests)
endif()
add_executable(NxResearchGraphExportSample Tools/ResearchGraph/NxResearchGraphExportSample.c)
target_link_libraries(NxResearchGraphExportSample PRIVATE NexioraNCP)

# ============================================================
# EPIC-0005 — First Autonomous Research Execution
# ============================================================

if(NEXIORA_BUILD_TESTS)
    add_executable(NxAutonomousExecutionTests
        Tests/Unit/NxAutonomousExecutionTests.c
    )
    target_link_libraries(NxAutonomousExecutionTests PRIVATE NexioraNCP)
    add_test(NAME NxAutonomousExecutionTests COMMAND NxAutonomousExecutionTests)
endif()

if(NEXIORA_BUILD_TESTS)
    add_executable(NxResearchDashboardTests
        Tests/Unit/NxResearchDashboardTests.c
    )
    target_link_libraries(NxResearchDashboardTests PRIVATE NexioraNCP)
    add_test(NAME NxResearchDashboardTests COMMAND NxResearchDashboardTests)
endif()




# ============================================================
# EPIC-0009 — Knowledge Acquisition Engine
# ============================================================

add_executable(nexiora_acquire
    Tools/Knowledge/NxKnowledgeAcquireTool.c
)
target_link_libraries(nexiora_acquire PRIVATE NexioraNCP)
if(NEXIORA_BUILD_TESTS)
    add_executable(NxKnowledgeAcquisitionTests
        Tests/Unit/NxKnowledgeAcquisitionTests.c
    )
    target_link_libraries(NxKnowledgeAcquisitionTests PRIVATE NexioraNCP)
    add_test(NAME NxKnowledgeAcquisitionTests COMMAND NxKnowledgeAcquisitionTests)
endif()


# ============================================================
# EPIC-0010 — Research Progress Framework
# ============================================================

add_executable(nexiora_progress_demo
    Tools/Research/NxProgressDemo.c
)
target_link_libraries(nexiora_progress_demo PRIVATE NexioraNCP)


if(NEXIORA_BUILD_TESTS)
    add_executable(NxProgressEngineTests
        Tests/Unit/NxProgressEngineTests.c
    )
    target_link_libraries(NxProgressEngineTests PRIVATE NexioraNCP)
    add_test(NAME NxProgressEngineTests COMMAND NxProgressEngineTests)
endif()



# ============================================================
# HIST-0003 — First Learning Cycle
# ============================================================

if(NEXIORA_BUILD_TESTS)
    add_executable(NxKnowledgeStoreTests
        Tests/Unit/NxKnowledgeStoreTests.c
    )
    target_link_libraries(NxKnowledgeStoreTests PRIVATE NexioraNCP)
    add_test(NAME NxKnowledgeStoreTests COMMAND NxKnowledgeStoreTests)

    add_executable(NxTopicInvestigationTests
        Tests/Unit/NxTopicInvestigationTests.c
    )
    target_link_libraries(NxTopicInvestigationTests PRIVATE NexioraNCP)
    add_test(NAME NxTopicInvestigationTests COMMAND NxTopicInvestigationTests)
endif()
# ============================================================
# CORE-0001 — Real Learning Core
# ============================================================

if(NEXIORA_BUILD_TESTS)
    add_executable(NxLearningCoreTests
        Tests/Unit/NxLearningCoreTests.c
    )
    target_link_libraries(NxLearningCoreTests PRIVATE NexioraNCP)
    add_test(NAME NxLearningCoreTests COMMAND NxLearningCoreTests)
endif()


# ============================================================
# CORE-0003 — Topic Question Answering
# ============================================================
if(NEXIORA_BUILD_TESTS)
    add_executable(NxTopicQuestionTests
        Tests/Unit/NxTopicQuestionTests.c
    )
    target_link_libraries(NxTopicQuestionTests PRIVATE NexioraNCP)
    add_test(NAME NxTopicQuestionTests COMMAND NxTopicQuestionTests)
endif()

# ============================================================
# COGNITIVE-0001 — Cognitive Core: local file ingestion + QA
# ============================================================

target_sources(NexioraNCP PRIVATE
    Source/Cognitive/NxCognitiveCore.c
)

add_executable(nexiora_cognitive
    Tools/Cognitive/NxCognitiveTool.c
)
target_link_libraries(nexiora_cognitive PRIVATE NexioraNCP)

if(NEXIORA_BUILD_TESTS)
    add_executable(NxCognitiveCoreTests
        Tests/Unit/NxCognitiveCoreTests.c
    )
    target_link_libraries(NxCognitiveCoreTests PRIVATE NexioraNCP)
    add_test(NAME NxCognitiveCoreTests COMMAND NxCognitiveCoreTests)
endif()

# COGNITIVE-0002: Cognitive directory ingest tool
if(NOT TARGET nexiora_cognitive_batch)
    add_executable(nexiora_cognitive_batch
        Tools/Cognitive/NxCognitiveBatchTool.c
    )
    target_link_libraries(nexiora_cognitive_batch PRIVATE NexioraNCP)
    set_target_properties(nexiora_cognitive_batch PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endif()

# ============================================================
# NCOS-001 — Session Engine
# ============================================================

target_sources(NexioraNCP PRIVATE
    Source/NCOS/NxSessionEngine.c
)

add_executable(nexiora_session
    Tools/NCOS/NxSessionTool.c
)
target_link_libraries(nexiora_session PRIVATE NexioraNCP)

if(NEXIORA_BUILD_TESTS)
    add_executable(NxSessionEngineTests
        Tests/Unit/NxSessionEngineTests.c
    )
    target_link_libraries(NxSessionEngineTests PRIVATE NexioraNCP)
    add_test(NAME NxSessionEngineTests COMMAND NxSessionEngineTests)
endif()

# ============================================================
# NCOS-002 — Planning Engine
# ============================================================

target_sources(NexioraNCP PRIVATE
    Source/NCOS/NxPlanningEngine.c
)

add_executable(nexiora_plan
    Tools/NCOS/NxPlanningTool.c
)
target_link_libraries(nexiora_plan PRIVATE NexioraNCP)

if(NEXIORA_BUILD_TESTS)
    add_executable(NxPlanningEngineTests
        Tests/Unit/NxPlanningEngineTests.c
    )
    target_link_libraries(NxPlanningEngineTests PRIVATE NexioraNCP)
    add_test(NAME NxPlanningEngineTests COMMAND NxPlanningEngineTests)
endif()

# ============================================================
# NCOS-003 — Concept Registry
# ============================================================

target_sources(NexioraNCP PRIVATE
    Source/NCOS/NxConceptRegistry.c
)

add_executable(nexiora_concept
    Tools/NCOS/NxConceptTool.c
)
target_link_libraries(nexiora_concept PRIVATE NexioraNCP)

if(NEXIORA_BUILD_TESTS)
    add_executable(NxConceptRegistryTests
        Tests/Unit/NxConceptRegistryTests.c
    )
    target_link_libraries(NxConceptRegistryTests PRIVATE NexioraNCP)
    add_test(NAME NxConceptRegistryTests COMMAND NxConceptRegistryTests)
endif()

# ============================================================
# NCOS-004 — Concept Graph
# ============================================================

if(NOT TARGET nexiora_graph)
    target_sources(NexioraNCP PRIVATE
        Source/NCOS/NxConceptGraph.c
    )

    add_executable(nexiora_graph
        Tools/NCOS/NxConceptGraphTool.c
    )
    target_link_libraries(nexiora_graph PRIVATE NexioraNCP)

    if(NEXIORA_BUILD_TESTS)
        add_executable(NxConceptGraphTests
            Tests/Unit/NxConceptGraphTests.c
        )
        target_link_libraries(NxConceptGraphTests PRIVATE NexioraNCP)
        add_test(NAME NxConceptGraphTests COMMAND NxConceptGraphTests)
        set_tests_properties(NxConceptGraphTests PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    endif()
endif()

# ============================================================
# NCOS-005 — Graph Reasoning
# ============================================================

if(NOT TARGET nexiora_reason)
    target_sources(NexioraNCP PRIVATE
        Source/NCOS/NxGraphReasoner.c
    )

    add_executable(nexiora_reason
        Tools/NCOS/NxGraphReasonerTool.c
    )
    target_link_libraries(nexiora_reason PRIVATE NexioraNCP)

    if(NEXIORA_BUILD_TESTS)
        add_executable(NxGraphReasonerTests
            Tests/Unit/NxGraphReasonerTests.c
        )
        target_link_libraries(NxGraphReasonerTests PRIVATE NexioraNCP)
        add_test(NAME NxGraphReasonerTests COMMAND NxGraphReasonerTests)
        set_tests_properties(NxGraphReasonerTests PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    endif()
endif()

# NCOS-006: Answer Composer
if(TARGET NexioraNCP)
    target_sources(NexioraNCP PRIVATE Source/NCOS/NxAnswerComposer.c)
endif()

add_executable(NxAnswerComposerTests Tests/Unit/NxAnswerComposerTests.c)
target_link_libraries(NxAnswerComposerTests PRIVATE NexioraNCP)
add_test(NAME NxAnswerComposerTests COMMAND NxAnswerComposerTests)
set_target_properties(NxAnswerComposerTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(nexiora_answer Source/Tools/nexiora_answer.c)
target_link_libraries(nexiora_answer PRIVATE NexioraNCP)
set_target_properties(nexiora_answer PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# NCOS-007: Intent-to-Plan Bridge
if(TARGET NexioraNCP)
    target_sources(NexioraNCP PRIVATE Source/NCOS/NxIntentPlanner.c)
endif()

add_executable(NxIntentPlannerTests Tests/Unit/NxIntentPlannerTests.c)
target_link_libraries(NxIntentPlannerTests PRIVATE NexioraNCP)
add_test(NAME NxIntentPlannerTests COMMAND NxIntentPlannerTests)
set_target_properties(NxIntentPlannerTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(nexiora_intent_plan Source/Tools/nexiora_intent_plan.c)
target_link_libraries(nexiora_intent_plan PRIVATE NexioraNCP)
set_target_properties(nexiora_intent_plan PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ============================================================
# NCOS-008 — Task Execution Engine
# ============================================================
target_sources(NexioraNCP PRIVATE
    Source/NCOS/NxTaskExecutionEngine.c
)

add_executable(nexiora_task
    Tools/NCOS/NxTaskTool.c
)
target_link_libraries(nexiora_task PRIVATE NexioraNCP)

if(NEXIORA_BUILD_TESTS)
    add_executable(NxTaskExecutionEngineTests
        Tests/Unit/NxTaskExecutionEngineTests.c
    )
    target_link_libraries(NxTaskExecutionEngineTests PRIVATE NexioraNCP)
    add_test(NAME NxTaskExecutionEngineTests COMMAND NxTaskExecutionEngineTests)
endif()
# ============================================================
# NCOS-009 — Compiler Engine
# ============================================================

add_executable(nexiora_compile
    Tools/NCOS/NxCompilerTool.c
)
target_link_libraries(nexiora_compile PRIVATE NexioraNCP)

if(NEXIORA_BUILD_TESTS)
    add_executable(NxCompilerEngineTests
        Tests/Unit/NxCompilerEngineTests.c
    )
    target_link_libraries(NxCompilerEngineTests PRIVATE NexioraNCP)
    add_test(NAME NxCompilerEngineTests COMMAND NxCompilerEngineTests)
endif()
# ============================================================
# NCOS-010 — Build Error Analyzer
# ============================================================

add_executable(nexiora_analyze_log
    Tools/NCOS/NxBuildLogAnalyzerTool.c
)
target_link_libraries(nexiora_analyze_log PRIVATE NexioraNCP)

if(NEXIORA_BUILD_TESTS)
    add_executable(NxBuildLogAnalyzerTests
        Tests/Unit/NxBuildLogAnalyzerTests.c
    )
    target_link_libraries(NxBuildLogAnalyzerTests PRIVATE NexioraNCP)
    add_test(NAME NxBuildLogAnalyzerTests COMMAND NxBuildLogAnalyzerTests)
endif()



# NCOS-011 Auto-Fix Proposal Engine
add_executable(nexiora_fix_proposal Tools/nexiora_fix_proposal.c)
target_link_libraries(nexiora_fix_proposal PRIVATE NexioraNCP)
set_target_properties(nexiora_fix_proposal PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(NxAutoFixProposalEngineTests Tests/Unit/NxAutoFixProposalEngineTests.c)
target_link_libraries(NxAutoFixProposalEngineTests PRIVATE NexioraNCP)
set_target_properties(NxAutoFixProposalEngineTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_test(NAME NxAutoFixProposalEngineTests COMMAND NxAutoFixProposalEngineTests)

# NCOS-012 Safe Patch Engine
target_sources(NexioraNCP PRIVATE
    Source/NCOS/NxSafePatchEngine.c
)

add_executable(nexiora_safe_patch Tools/nexiora_safe_patch.c)
target_link_libraries(nexiora_safe_patch PRIVATE NexioraNCP)

if(NEXIORA_BUILD_TESTS)
    add_executable(NxSafePatchEngineTests Tests/Unit/NxSafePatchEngineTests.c)
    target_link_libraries(NxSafePatchEngineTests PRIVATE NexioraNCP)
    add_test(NAME NxSafePatchEngineTests COMMAND NxSafePatchEngineTests)
endif()

# NCOS-013 Patch Approval Engine
target_sources(NexioraNCP PRIVATE
    Source/NCOS/NxPatchApprovalEngine.c
)

add_executable(nexiora_patch_approval Tools/nexiora_patch_approval.c)
target_link_libraries(nexiora_patch_approval PRIVATE NexioraNCP)

if(NEXIORA_BUILD_TESTS)
    add_executable(NxPatchApprovalEngineTests Tests/Unit/NxPatchApprovalEngineTests.c)
    target_link_libraries(NxPatchApprovalEngineTests PRIVATE NexioraNCP)
    add_test(NAME NxPatchApprovalEngineTests COMMAND NxPatchApprovalEngineTests)
endif()

# NCOS-014 Approved Patch Apply Engine
target_sources(NexioraNCP PRIVATE
    Source/NCOS/NxApprovedPatchApplyEngine.c
    Source/NCOS/NxPatchVerificationGate.c
)

add_executable(nexiora_apply_approved_patch Tools/nexiora_apply_approved_patch.c)
target_link_libraries(nexiora_apply_approved_patch PRIVATE NexioraNCP)

if(NEXIORA_BUILD_TESTS)
    add_executable(NxApprovedPatchApplyEngineTests Tests/Unit/NxApprovedPatchApplyEngineTests.c)
    target_link_libraries(NxApprovedPatchApplyEngineTests PRIVATE NexioraNCP)
    add_test(NAME NxApprovedPatchApplyEngineTests COMMAND NxApprovedPatchApplyEngineTests)
endif()

add_executable(nexiora_patch_verify
    Tools/nexiora_patch_verify.c
)
target_link_libraries(nexiora_patch_verify PRIVATE NexioraNCP)

add_executable(NxPatchVerificationGateTests
    Tests/Unit/NxPatchVerificationGateTests.c
)
target_link_libraries(NxPatchVerificationGateTests PRIVATE NexioraNCP)
add_test(NAME NxPatchVerificationGateTests COMMAND NxPatchVerificationGateTests)

# NCOS-016 - Nexiora Package Manager
target_sources(NexioraNCP PRIVATE
    Source/NCOS/NxPackageManager.c
    Source/NCOS/NxPackageApply.c
)

add_executable(nexiora_package
    Tools/NCOS/NxPackageTool.c
)
target_link_libraries(nexiora_package PRIVATE NexioraNCP)

if(NEXIORA_BUILD_TESTS)
    add_executable(NxPackageManagerTests
        Tests/Unit/NxPackageManagerTests.c
    )
    target_link_libraries(NxPackageManagerTests PRIVATE NexioraNCP)
    add_test(NAME NxPackageManagerTests COMMAND NxPackageManagerTests)
endif()

# EPIC-01 Sprint-19 - Autonomous Workspace Engine
target_sources(NexioraNCP PRIVATE
    Source/NCOS/NxWorkspaceEngine.c
)

add_executable(nexiora_workspace
    Tools/NCOS/NxWorkspaceTool.c
)
target_link_libraries(nexiora_workspace PRIVATE NexioraNCP)

if(NEXIORA_BUILD_TESTS)
    add_executable(NxWorkspaceEngineTests
        Tests/Unit/NxWorkspaceEngineTests.c
    )
    target_link_libraries(NxWorkspaceEngineTests PRIVATE NexioraNCP)
    add_test(NAME NxWorkspaceEngineTests COMMAND NxWorkspaceEngineTests)
endif()


# EPIC-03 Sprint-01 - Multimodal Ingestion Foundation
target_sources(NexioraNCP PRIVATE
    Source/Multimodal/NxMultimodalIngestion.c
)
add_executable(nexiora_ingest Tools/Multimodal/NxMultimodalIngestTool.c)
target_link_libraries(nexiora_ingest PRIVATE NexioraNCP)
set_target_properties(nexiora_ingest PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
if(NEXIORA_BUILD_TESTS)
    add_executable(NxMultimodalIngestionTests Tests/Unit/NxMultimodalIngestionTests.c)
    target_link_libraries(NxMultimodalIngestionTests PRIVATE NexioraNCP)
    set_target_properties(NxMultimodalIngestionTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    add_test(NAME NxMultimodalIngestionTests COMMAND NxMultimodalIngestionTests)
endif()

# ============================================================
# EPIC-03 Sprint-02 — Decoder and OCR Adapters
# ============================================================
target_sources(NexioraNCP PRIVATE
    Source/Multimodal/NxDecoderAdapters.c
)

add_executable(nexiora_decode
    Tools/Multimodal/NxDecoderTool.c
)
target_link_libraries(nexiora_decode PRIVATE NexioraNCP)

if(NEXIORA_BUILD_TESTS)
    add_executable(NxDecoderAdaptersTests
        Tests/Unit/NxDecoderAdaptersTests.c
    )
    target_link_libraries(NxDecoderAdaptersTests PRIVATE NexioraNCP)
    add_test(NAME NxDecoderAdaptersTests COMMAND NxDecoderAdaptersTests)
    set_tests_properties(NxDecoderAdaptersTests PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

# EPIC-03 Sprint-03 - Multimodal Cognitive Integration Engine
target_sources(NexioraNCP PRIVATE
    Source/Multimodal/NxMultimodalCognitiveIntegration.c
)

add_executable(nexiora_multimodal_cognitive
    Tools/Multimodal/NxMultimodalCognitiveTool.c
)
target_link_libraries(nexiora_multimodal_cognitive PRIVATE NexioraNCP)
set_target_properties(nexiora_multimodal_cognitive PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(NEXIORA_BUILD_TESTS)
    add_executable(NxMultimodalCognitiveIntegrationTests
        Tests/Unit/NxMultimodalCognitiveIntegrationTests.c
    )
    target_link_libraries(NxMultimodalCognitiveIntegrationTests PRIVATE NexioraNCP)
    set_target_properties(NxMultimodalCognitiveIntegrationTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    add_test(NAME NxMultimodalCognitiveIntegrationTests COMMAND NxMultimodalCognitiveIntegrationTests)
    set_tests_properties(NxMultimodalCognitiveIntegrationTests PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

# CAPABILITY-01 Sprint-01 - Web Cognitive Pipeline Foundation
target_sources(NexioraNCP PRIVATE
    Source/CognitivePipeline/NxWebCognitivePipeline.c
)

add_executable(nexiora_web_cognitive
    Tools/CognitivePipeline/NxWebCognitivePipelineTool.c
)
target_link_libraries(nexiora_web_cognitive PRIVATE NexioraNCP)
set_target_properties(nexiora_web_cognitive PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(NEXIORA_BUILD_TESTS)
    add_executable(NxWebCognitivePipelineTests
        Tests/Unit/NxWebCognitivePipelineTests.c
    )
    target_link_libraries(NxWebCognitivePipelineTests PRIVATE NexioraNCP)
    set_target_properties(NxWebCognitivePipelineTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    add_test(NAME NxWebCognitivePipelineTests COMMAND NxWebCognitivePipelineTests)
    set_tests_properties(NxWebCognitivePipelineTests PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

# CAPABILITY-02 Sprint-01 - Grounded Multi-Evidence Reasoning
target_sources(NexioraNCP PRIVATE
    Source/Reasoning/NxGroundedReasoning.c
)
add_executable(nexiora_reason_grounded
    Tools/Reasoning/NxGroundedReasoningTool.c
)
target_link_libraries(nexiora_reason_grounded PRIVATE NexioraNCP)
set_target_properties(nexiora_reason_grounded PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
if(NEXIORA_BUILD_TESTS)
    add_executable(NxGroundedReasoningTests
        Tests/Unit/NxGroundedReasoningTests.c
    )
    target_link_libraries(NxGroundedReasoningTests PRIVATE NexioraNCP)
    set_target_properties(NxGroundedReasoningTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    add_test(NAME NxGroundedReasoningTests COMMAND NxGroundedReasoningTests)
    set_tests_properties(NxGroundedReasoningTests PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()


# CAPABILITY-02 Sprint-01 Repair1: materialize every registered test binary
if(NEXIORA_BUILD_TESTS)
    set(NEXIORA_FULL_TEST_TARGETS
        nexiora_tests
        NxVectorTests
        NxResearchTests
        NxJournalTests
        NxExperimentRunnerTests
        NxResearchEvidenceTests
        NxPromotionTests
        NxAutonomousResearchPipelineTests
        NxDiscoveryEngineTests
        NxResearchGraphTests
        NxResearchGraphExporterTests
        NxResearchGraphQueryTests
        NxResearchSchedulerTests
        NxResearchQueueTests
        NxResearchPolicyTests
        NxResearchSessionTests
        NxAutonomousResearchLoopTests
        NxKnowledgeBaseTests
        NxKnowledgeQueryTests
        NxHypothesisEngineTests
        NxReasoningEngineTests
        NxEvidenceCorrelationTests
        NxConflictDetectorTests
        NxPersistentMemoryTests
        NxConversationTests
        NxAutonomousExecutionTests
        NxResearchDashboardTests
        NxKnowledgeAcquisitionTests
        NxProgressEngineTests
        NxKnowledgeStoreTests
        NxTopicInvestigationTests
        NxLearningCoreTests
        NxTopicQuestionTests
        NxCognitiveCoreTests
        NxSessionEngineTests
        NxPlanningEngineTests
        NxConceptRegistryTests
        NxConceptGraphTests
        NxGraphReasonerTests
        NxAnswerComposerTests
        NxIntentPlannerTests
        NxTaskExecutionEngineTests
        NxCompilerEngineTests
        NxBuildLogAnalyzerTests
        NxAutoFixProposalEngineTests
        NxSafePatchEngineTests
        NxPatchApprovalEngineTests
        NxApprovedPatchApplyEngineTests
        NxPatchVerificationGateTests
        NxPackageManagerTests
        NxWorkspaceEngineTests
        NxMultimodalIngestionTests
        NxDecoderAdaptersTests
        NxMultimodalCognitiveIntegrationTests
        NxWebCognitivePipelineTests
        NxGroundedReasoningTests
    )

    add_custom_target(nexiora_test_suite
        DEPENDS ${NEXIORA_FULL_TEST_TARGETS}
        COMMENT "Materializing all Nexiora test executables before CTest"
    )
endif()

# CAPABILITY-02 Sprint-02 — Contradiction and Hypothesis Reasoning
if(TARGET NexioraNCP)
    target_sources(NexioraNCP PRIVATE Source/Reasoning/NxContradictionHypothesis.c)
endif()
add_executable(nexiora_hypothesis Tools/Reasoning/NxContradictionHypothesisTool.c)
target_link_libraries(nexiora_hypothesis PRIVATE NexioraNCP)
if(NEXIORA_BUILD_TESTS)
    add_executable(NxContradictionHypothesisTests Tests/Unit/NxContradictionHypothesisTests.c)
    target_link_libraries(NxContradictionHypothesisTests PRIVATE NexioraNCP)
    add_test(NAME NxContradictionHypothesisTests COMMAND NxContradictionHypothesisTests)
    set_tests_properties(NxContradictionHypothesisTests PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    if(TARGET nexiora_test_suite)
        add_dependencies(nexiora_test_suite NxContradictionHypothesisTests)
    endif()
endif()

# CAPABILITY-02 Sprint-03 — Conversational Memory and Context
if(TARGET NexioraNCP)
    target_sources(NexioraNCP PRIVATE Source/Reasoning/NxConversationalContext.c)
endif()
add_executable(nexiora_dialogue Tools/Reasoning/NxConversationalContextTool.c)
target_link_libraries(nexiora_dialogue PRIVATE NexioraNCP)
set_target_properties(nexiora_dialogue PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
if(NEXIORA_BUILD_TESTS)
    add_executable(NxConversationalContextTests Tests/Unit/NxConversationalContextTests.c)
    target_link_libraries(NxConversationalContextTests PRIVATE NexioraNCP)
    set_target_properties(NxConversationalContextTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    add_test(NAME NxConversationalContextTests COMMAND NxConversationalContextTests)
    set_tests_properties(NxConversationalContextTests PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    if(TARGET nexiora_test_suite)
        add_dependencies(nexiora_test_suite NxConversationalContextTests)
    endif()
endif()

# NCOS-023 Repair3 - staged Package Manager bootstrap without premature replacement
if(WIN32)
    add_executable(nexiora_package_next
        Tools/NCOS/NxPackageTool.c
    )
    target_link_libraries(nexiora_package_next PRIVATE NexioraNCP)
    set_target_properties(nexiora_package_next PROPERTIES
        OUTPUT_NAME "nexiora_package.next"
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )

    add_executable(nexiora_self_update
        Tools/NCOS/NxPackageSelfUpdate.c
    )
    set_target_properties(nexiora_self_update PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )

    if(TARGET nexiora_test_suite)
        add_custom_target(nexiora_test_suite_staged
            DEPENDS nexiora_test_suite nexiora_package_next nexiora_self_update
            COMMENT "Certifying all tests and staging Package Manager self-update"
        )
    endif()
endif()


# CAPABILITY-02 Sprint-04 — Autonomous Knowledge Gap Research
if(TARGET NexioraNCP)
    target_sources(NexioraNCP PRIVATE Source/Research/NxKnowledgeGapResearch.c)
endif()
add_executable(nexiora_gap Tools/Research/NxKnowledgeGapResearchTool.c)
target_link_libraries(nexiora_gap PRIVATE NexioraNCP)
set_target_properties(nexiora_gap PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
if(NEXIORA_BUILD_TESTS)
    add_executable(NxKnowledgeGapResearchTests Tests/Unit/NxKnowledgeGapResearchTests.c)
    target_link_libraries(NxKnowledgeGapResearchTests PRIVATE NexioraNCP)
    set_target_properties(NxKnowledgeGapResearchTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    add_test(NAME NxKnowledgeGapResearchTests COMMAND NxKnowledgeGapResearchTests)
    set_tests_properties(NxKnowledgeGapResearchTests PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    if(TARGET nexiora_test_suite)
        add_dependencies(nexiora_test_suite NxKnowledgeGapResearchTests nexiora_gap)
    endif()
    if(TARGET nexiora_test_suite_staged)
        add_dependencies(nexiora_test_suite_staged NxKnowledgeGapResearchTests nexiora_gap)
    endif()
endif()

# CAPABILITY-02 Sprint-05 — Unified Cognitive Dialogue
if(TARGET NexioraNCP)
    target_sources(NexioraNCP PRIVATE Source/Reasoning/NxUnifiedCognitiveDialogue.c)
endif()
add_executable(nexiora_cognitive_dialogue Tools/Reasoning/NxUnifiedCognitiveDialogueTool.c)
target_link_libraries(nexiora_cognitive_dialogue PRIVATE NexioraNCP)
set_target_properties(nexiora_cognitive_dialogue PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
if(NEXIORA_BUILD_TESTS)
    add_executable(NxUnifiedCognitiveDialogueTests Tests/Unit/NxUnifiedCognitiveDialogueTests.c)
    target_link_libraries(NxUnifiedCognitiveDialogueTests PRIVATE NexioraNCP)
    set_target_properties(NxUnifiedCognitiveDialogueTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    add_test(NAME NxUnifiedCognitiveDialogueTests COMMAND NxUnifiedCognitiveDialogueTests)
    set_tests_properties(NxUnifiedCognitiveDialogueTests PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    if(TARGET nexiora_test_suite)
        add_dependencies(nexiora_test_suite NxUnifiedCognitiveDialogueTests nexiora_cognitive_dialogue)
    endif()
    if(TARGET nexiora_test_suite_staged)
        add_dependencies(nexiora_test_suite_staged NxUnifiedCognitiveDialogueTests nexiora_cognitive_dialogue)
    endif()
endif()

# NCOS-025 — Installation Certification and QA Evidence
if(TARGET NexioraNCP)
    target_sources(NexioraNCP PRIVATE Source/NCOS/NxPackageCertification.c)
endif()

# NCOS-024 — Workspace State Manifest delivery certification
# The workspace CLI and its tests are mandatory artifacts of the Pure Apply build.
if(TARGET nexiora_test_suite)
    add_dependencies(nexiora_test_suite nexiora_workspace NxWorkspaceEngineTests)
endif()
if(TARGET nexiora_test_suite_staged)
    add_dependencies(nexiora_test_suite_staged nexiora_workspace NxWorkspaceEngineTests)
endif()
