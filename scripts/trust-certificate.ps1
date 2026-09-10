# Trust the self-signed certificate for the MSYS2 UCRT64 Shell package.
# ! Shared by BOTH options (A: sparse, B: full package).
# ! NEEDS ELEVATED PRIVILEGES TO RUN.
Import-PfxCertificate -FilePath .\scripts\Msys2Ucrt64Shell.pfx -CertStoreLocation Cert:\LocalMachine\Root -Password $pwd
