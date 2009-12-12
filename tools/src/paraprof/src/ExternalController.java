package edu.uoregon.tau.paraprof;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.sql.SQLException;
import java.util.Iterator;
import java.util.List;
import java.util.StringTokenizer;

import edu.uoregon.tau.perfdmf.*;

public class ExternalController {

    static public void runController() {
        try {
            System.out.println("Control Mode Active!");

            BufferedReader stdin = new BufferedReader(new InputStreamReader(System.in));

            String input = stdin.readLine();

            while (input != null) {
                System.out.println("got input: " + input);

                if (input.startsWith("control ")) {
                    processCommand(input.substring(8));
                }
                input = stdin.readLine();
            }

            exitController();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    static public void exitController() {
        System.out.println("Control Mode Complete!");
        System.exit(0);

    }

    static public void processCommand(String command) throws SQLException {
        System.out.println("processing command: " + command);
        if (command.equals("open manager")) {
            ParaProf.paraProfManagerWindow.setVisible(true);
        } else if (command.equals("list databases")) {
            listDatabases();
        } else if (command.startsWith("list applications")) {
            listApplications(command.substring("list applications".length() + 1));
        } else if (command.startsWith("list experiments")) {
            listExperiments(command.substring("list experiments".length() + 1));
        } else if (command.startsWith("list trials")) {
            listTrials(command.substring("list trials".length() + 1));
        } else if (command.equals("exit")) {
            exitController();
        }

    }

    static public void listApplications(String databaseID) throws SQLException {
        int id = Integer.parseInt(databaseID);
        List databases = Database.getDatabases();

        DatabaseAPI databaseAPI = new DatabaseAPI();
        databaseAPI.initialize((Database) databases.get(id));
        List apps = databaseAPI.getApplicationList();
        for (Iterator it = apps.iterator(); it.hasNext();) {
            Application app = (Application) it.next();
            outputCommand("return " + app.getID() + " " + app.getName());
        }
        outputCommand("endreturn");
    }

    static public void listExperiments(String ids) throws SQLException {

        StringTokenizer tokenizer = new StringTokenizer(ids, " ");

        int dbID = Integer.parseInt(tokenizer.nextToken());
        int appID = Integer.parseInt(tokenizer.nextToken());

        DatabaseAPI databaseAPI = new DatabaseAPI();
        databaseAPI.initialize((Database) Database.getDatabases().get(dbID));
        databaseAPI.setApplication(appID);

        List exps = databaseAPI.getExperimentList();
        for (Iterator it = exps.iterator(); it.hasNext();) {
            Experiment exp = (Experiment) it.next();
            outputCommand("return " + exp.getID() + " " + exp.getName());
        }
        outputCommand("endreturn");
    }

    static public void listTrials(String ids) throws SQLException {

        StringTokenizer tokenizer = new StringTokenizer(ids, " ");

        int dbID = Integer.parseInt(tokenizer.nextToken());
        int expID = Integer.parseInt(tokenizer.nextToken());

        DatabaseAPI databaseAPI = new DatabaseAPI();
        databaseAPI.initialize((Database) Database.getDatabases().get(dbID));
        databaseAPI.setExperiment(expID);

        List trials = databaseAPI.getTrialList(false);
        for (Iterator it = trials.iterator(); it.hasNext();) {
            Trial trial = (Trial) it.next();
            outputCommand("return " +trial.getID() + " " + trial.getName());
        }
        outputCommand("endreturn");
    }

    static public void listDatabases() {
        List databases = Database.getDatabases();
        int id = 0;
        for (Iterator it = databases.iterator(); it.hasNext();) {
            Database db = (Database) it.next();
            outputCommand("return " + id + " " + db.getName());
            id++;
        }
        outputCommand("endreturn");

    }

    static public void outputCommand(String command) {
        System.out.println("control " + command);
    }

}
