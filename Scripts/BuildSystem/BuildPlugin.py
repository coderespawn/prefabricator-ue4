import sys
import os
import subprocess

if not "BUILD_UE4_PLUGIN" in os.environ or os.environ["BUILD_UE4_PLUGIN"] == "":
    print("ERROR: Environment variable BUILD_UE4_PLUGIN doesn't exist")
    sys.exit()

if not os.path.exists(os.environ["BUILD_UE4_PLUGIN"]):
    print("ERROR: Invalid build file specified in environment variable BUILD_UE4_PLUGIN")
    sys.exit()

args = ["python", os.environ["BUILD_UE4_PLUGIN"], "--find_descriptor"]
if len(sys.argv) >= 2:
    args.append(sys.argv[1])
args.append("../../")

process = subprocess.Popen(args)
process.wait()
