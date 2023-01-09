# G2 - Operative Systems Project 1
Allows communication between Arda servers and IluvatarSons.

## Compilation & execution
Download the project and compile in LaSalle servers (matagalls, montserrat or puigpedros) with `make` command.

Once compiled, 2 executable files will be created: IluvatarSon and Arda.
### Execute Arda server
1. Create a file for Arda with the following format
```
<Arda name>
<IP address>
<port>
```

2. Issue the command:
```
./Arda files/<arda_file>.<ext>
```
### Execute Iluvatar
1. Create a file for Arda with the following format
```
<Iluvatar name>
/<directory>
<server IP address>
<server port>
<Iluvatar IP address>
<Iluvatar port>
```

2. Issue the command:
```
./IluvatarSon files/<iluvatar_file>.<ext>
```
3. Enter any of the followinf commands
* LIST USERS
* UPDATE USERS
* SEND MSG user msg
* SEND FILE user file
* EXIT

## Testing
We provide some configuration files for Arda and IluvatarSons (found in the "files" directory) as well as the IluvatarSons directories.

## Authors
* Ángel García Gascón
* Claudia Lajara Silvosa
