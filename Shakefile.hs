import Development.Shake
import Development.Shake.Command
import Development.Shake.FilePath
import Development.Shake.Util
import System.Console.GetOpt


main :: IO()
main = shakeArgs shakeOptions{shakeFiles="_build"} $ do
  want ["_build/main" <.> exe]

  phony "clean" $ do
    putInfo "Cleaning files in build"
    removeFilesAfter "_build" ["//*"]

  "_build/main" <.> exe %> \out -> do
    cs <- getDirectoryFiles "" ["src//*.c"]
    let os = ["_build" </> c -<.> "o" | c <- cs]
    need os
    cmd_ "gcc" "-Iinclude" "main.c" "-o" [out] os

  "_build//*.o" %> \out -> do
    let c = dropDirectory1 out -<.> "c"
    let m = out -<.> "m"
    cmd_ "gcc" "-Iinclude" "-c" [c] "-o" [out] "-MMD -MF" [m]
    needMakefileDependencies m
