// Import the utility functionality.
// import jobs.generation.Utilities;
import jobs.generation.InternalUtilities;

// Defines the name of the repo, used elsewhere in the file
def project = 'Microsoft/ChakraCorePrivate'

// Generate the builds for debug and release, commit and PRJob
[true, false].each { isPR -> // Defines a closure over true and false, value assigned to isPR
    ['x86', 'x64'].each { buildArch -> // build both architectures
        ['debug', 'test', 'release'].each { buildType -> // build all three configurations
            // Determine the name for the new job.  The first parameter is the project,
            // the second parameter is the base name for the job, and the last parameter
            // is a boolean indicating whether the job will be a PR job.  If true, the
            // suffix _prtest will be appended.
            def config = "${buildArch}_${buildType}"
            def jobName = InternalUtilities.getFullJobName(project, config, isPR)

            // Define build string
            def buildString = "call jenkins.buildone.cmd ${buildArch} ${buildType}"

            def testString = "call jenkins.testone.cmd ${buildArch} ${buildType}"
            def testableConfig = buildType in ['debug', 'test']

            // Create a new job with the specified name.  The brace opens a new closure
            // and calls made within that closure apply to the newly created job.
            def newJob = job(jobName) {
                label('windows') // run on Windows

                // This opens the set of build steps that will be run.
                steps {
                    batchFile(buildString) // run the parameter as a batch script
                    if (testableConfig) {
                        batchFile(testString)
                    }
                }
            }

            InternalUtilities.addPrivatePermissions(newJob)

            // This call performs remaining common job setup on the newly created job.
            // This is used most commonly for simple inner loop testing.
            // It does the following:
            //   1. Sets up source control for the project.
            //   2. Adds a push trigger if the job is a PR job
            //   3. Adds a github PR trigger if the job is a PR job.
            //      The optional context (label that you see on github in the PR checks) is added.
            //      If not provided the context defaults to the job name.
            //   4. Adds standard options for build retention and timeouts
            //   5. Adds standard parameters for PR and push jobs.
            //      These allow PR jobs to be used for simple private testing, for instance.
            // See the documentation for this function to see additional optional parameters.
            InternalUtilities.simpleInnerLoopJobSetup(newJob, project, isPR, "Windows ${config}")
        }
    }
}
