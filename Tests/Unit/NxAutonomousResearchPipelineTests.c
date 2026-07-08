#include "Nexiora/Research/NxAutonomousResearchPipeline.h"

#include <stdio.h>

typedef struct TestContext
{
    int step;
    int score;
    int fail_at_step;
} TestContext;

static int MaybeFail(TestContext* ctx, int expected_step)
{
    if (ctx->step != expected_step)
    {
        return 1;
    }

    if (ctx->fail_at_step == expected_step)
    {
        return 1;
    }

    ctx->step += 1;
    return 0;
}

static int Discover(void* user_data)
{
    return MaybeFail((TestContext*)user_data, 0);
}

static int Run(void* user_data)
{
    return MaybeFail((TestContext*)user_data, 1);
}

static int Evidence(void* user_data)
{
    return MaybeFail((TestContext*)user_data, 2);
}

static int Journal(void* user_data)
{
    return MaybeFail((TestContext*)user_data, 3);
}

static int Manifest(void* user_data)
{
    return MaybeFail((TestContext*)user_data, 4);
}

static int Promotion(void* user_data, int* score_out)
{
    TestContext* ctx = (TestContext*)user_data;

    if (score_out == 0)
    {
        return 1;
    }

    if (MaybeFail(ctx, 5) != 0)
    {
        return 1;
    }

    *score_out = ctx->score;
    return 0;
}

static NxAutonomousResearchPipelineCallbacks MakeCallbacks(void)
{
    NxAutonomousResearchPipelineCallbacks callbacks;

    callbacks.discover_experiments = Discover;
    callbacks.run_experiments = Run;
    callbacks.generate_evidence = Evidence;
    callbacks.write_journal = Journal;
    callbacks.update_manifest = Manifest;
    callbacks.evaluate_promotion = Promotion;

    return callbacks;
}

static int TestHappyPathReadyForHumanReview(void)
{
    TestContext ctx;
    NxAutonomousResearchPipelineCallbacks callbacks;
    NxAutonomousResearchPipelineResult result;
    NxArpStatus status;

    ctx.step = 0;
    ctx.score = 94;
    ctx.fail_at_step = -1;

    callbacks = MakeCallbacks();
    status = NxAutonomousResearchPipeline_Run(&callbacks, &ctx, &result);

    if (status != NX_ARP_STATUS_OK)
    {
        printf("Expected OK\n");
        return 1;
    }

    if (ctx.step != 6)
    {
        printf("Expected all pipeline steps to run\n");
        return 1;
    }

    if (result.promotion_score != 94)
    {
        printf("Expected score 94\n");
        return 1;
    }

    if (result.recommendation != NX_ARP_RECOMMENDATION_READY_FOR_HUMAN_REVIEW)
    {
        printf("Expected READY_FOR_HUMAN_REVIEW\n");
        return 1;
    }

    return 0;
}

static int TestNeedsMoreEvidence(void)
{
    TestContext ctx;
    NxAutonomousResearchPipelineCallbacks callbacks;
    NxAutonomousResearchPipelineResult result;
    NxArpStatus status;

    ctx.step = 0;
    ctx.score = 75;
    ctx.fail_at_step = -1;

    callbacks = MakeCallbacks();
    status = NxAutonomousResearchPipeline_Run(&callbacks, &ctx, &result);

    if (status != NX_ARP_STATUS_OK)
    {
        printf("Expected OK for score 75\n");
        return 1;
    }

    if (result.recommendation != NX_ARP_RECOMMENDATION_NEEDS_MORE_EVIDENCE)
    {
        printf("Expected NEEDS_MORE_EVIDENCE\n");
        return 1;
    }

    return 0;
}

static int TestReject(void)
{
    TestContext ctx;
    NxAutonomousResearchPipelineCallbacks callbacks;
    NxAutonomousResearchPipelineResult result;
    NxArpStatus status;

    ctx.step = 0;
    ctx.score = 40;
    ctx.fail_at_step = -1;

    callbacks = MakeCallbacks();
    status = NxAutonomousResearchPipeline_Run(&callbacks, &ctx, &result);

    if (status != NX_ARP_STATUS_OK)
    {
        printf("Expected OK for score 40\n");
        return 1;
    }

    if (result.recommendation != NX_ARP_RECOMMENDATION_REJECT)
    {
        printf("Expected REJECT\n");
        return 1;
    }

    return 0;
}

static int TestStopsOnEvidenceFailure(void)
{
    TestContext ctx;
    NxAutonomousResearchPipelineCallbacks callbacks;
    NxAutonomousResearchPipelineResult result;
    NxArpStatus status;

    ctx.step = 0;
    ctx.score = 94;
    ctx.fail_at_step = 2;

    callbacks = MakeCallbacks();
    status = NxAutonomousResearchPipeline_Run(&callbacks, &ctx, &result);

    if (status != NX_ARP_STATUS_EVIDENCE_FAILED)
    {
        printf("Expected EVIDENCE_FAILED\n");
        return 1;
    }

    if (ctx.step != 2)
    {
        printf("Expected pipeline to stop at evidence step\n");
        return 1;
    }

    if (result.recommendation != NX_ARP_RECOMMENDATION_NONE)
    {
        printf("Expected no recommendation after failure\n");
        return 1;
    }

    return 0;
}

static int TestInvalidArguments(void)
{
    NxAutonomousResearchPipelineCallbacks callbacks;
    NxAutonomousResearchPipelineResult result;
    NxArpStatus status;

    callbacks = MakeCallbacks();

    status = NxAutonomousResearchPipeline_Run(0, 0, &result);
    if (status != NX_ARP_STATUS_INVALID_ARGUMENT)
    {
        printf("Expected invalid argument for null callbacks\n");
        return 1;
    }

    status = NxAutonomousResearchPipeline_Run(&callbacks, 0, 0);
    if (status != NX_ARP_STATUS_INVALID_ARGUMENT)
    {
        printf("Expected invalid argument for null result\n");
        return 1;
    }

    callbacks.generate_evidence = 0;
    status = NxAutonomousResearchPipeline_Run(&callbacks, 0, &result);
    if (status != NX_ARP_STATUS_INVALID_ARGUMENT)
    {
        printf("Expected invalid argument for missing callback\n");
        return 1;
    }

    return 0;
}

int main(void)
{
    if (TestHappyPathReadyForHumanReview() != 0)
    {
        return 1;
    }

    if (TestNeedsMoreEvidence() != 0)
    {
        return 1;
    }

    if (TestReject() != 0)
    {
        return 1;
    }

    if (TestStopsOnEvidenceFailure() != 0)
    {
        return 1;
    }

    if (TestInvalidArguments() != 0)
    {
        return 1;
    }

    return 0;
}
