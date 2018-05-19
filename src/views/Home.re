[%bs.raw "require('../styles/home.css')"];

open Types;

type shops = {
  near: list(shop),
  preferred: list(shop)
};

type shops_resp = {shops};

type shop_action = {
  success: bool,
  error: option(list(string))
};

module Decoder = {
  let shops = json : shops_resp =>
    Json.Decode.{
      shops:
        json
        |> field("shops", shops =>
             {
               near:
                 shops
                 |> field(
                      "near",
                      list(item =>
                        {
                          id: item |> field("id", string),
                          name: item |> field("name", string),
                          picture: item |> field("picture", string)
                        }
                      )
                    ),
               preferred:
                 shops
                 |> field(
                      "preferred",
                      list(item =>
                        {
                          id: item |> field("id", string),
                          name: item |> field("name", string),
                          picture: item |> field("picture", string)
                        }
                      )
                    )
             }
           )
    };
  let shop_action = json =>
    Json.Decode.{
      success: json |> field("success", bool),
      error: json |> optional(field("error", list(string)))
    };
};

type data =
  | LOADING_LOCATION
  | LOCATION_LOADED(float, float)
  | LOCATION_FAILED
  | LOADING_SHOPS
  | LOADING_SHOPS_FAILED
  | SHOPS_LOADED(shops);

type selected_tab = string;

type state = {
  selected_tab,
  data
};

type action =
  | AC_LOADING_LOCATION
  | AC_LOCATION_LOADED(float, float)
  | AC_LOCATION_FAILED
  | AC_LOADING_SHOPS
  | AC_LOADING_SHOPS_FAILED
  | AC_SHOPS_LOADED(shops)
  | AC_CHANGE_SELECTED_TAB(string)
  | AC_SHOP_LIKED(string)
  | AC_SHOP_DISLIKED(string)
  | AC_SHOP_UNLIKED(string);

let component = ReasonReact.reducerComponent("Home");

let getToken = () => {
  let token = Dom.Storage.(getItem("token", localStorage));
  switch token {
  | Some(token) => token
  | _ => ""
  };
};

let fetch_shops = (~longitude, ~latitude) =>
  Fetch.fetchWithInit(
    "http://localhost:4000/api/shops?lantitude="
    ++ string_of_float(latitude)
    ++ "&longitude="
    ++ string_of_float(longitude),
    Fetch.RequestInit.make(
      ~headers=
        Fetch.HeadersInit.make({
          "Accept": "application/json",
          "Authorization": getToken()
        }),
      ()
    )
  );

let shop_action = (url: string) =>
  Fetch.fetchWithInit(
    url,
    Fetch.RequestInit.make(
      ~method_=Post,
      ~headers=
        Fetch.HeadersInit.make({
          "Accept": "application/json",
          "Authorization": getToken()
        }),
      ()
    )
  );

let clickHandler = (send, shop_id, btn_id) => {
  switch btn_id {
  | "like" =>
    Js.Promise.(
      shop_action("http://localhost:4000/api/shop/like/" ++ shop_id)
      |> then_(Fetch.Response.json)
      |> then_(resp => {
           let r = resp |> Decoder.shop_action;
           r.success ? send(AC_SHOP_LIKED(shop_id)) : ();
           resolve();
         })
      |> ignore
    );
    ();
  | "dislike" =>
    Js.Promise.(
      shop_action("http://localhost:4000/api/shop/dislike/" ++ shop_id)
      |> then_(Fetch.Response.json)
      |> then_(resp => {
           let r = resp |> Decoder.shop_action;
           r.success ? send(AC_SHOP_DISLIKED(shop_id)) : ();
           resolve();
         })
      |> ignore
    );
    ();
  | "remove" =>
    Js.Promise.(
      shop_action("http://localhost:4000/api/shop/unlike/" ++ shop_id)
      |> then_(Fetch.Response.json)
      |> then_(resp => {
           let r = resp |> Decoder.shop_action;
           r.success ? send(AC_SHOP_UNLIKED(shop_id)) : ();
           resolve();
         })
      |> ignore
    );
    ();
  | _ => ()
  };
  ();
};

let make = _children => {
  ...component,
  initialState: () => {data: LOADING_LOCATION, selected_tab: "near"},
  reducer: (action, state) =>
    switch action {
    | AC_LOADING_SHOPS => ReasonReact.Update({...state, data: LOADING_SHOPS})
    | AC_LOCATION_LOADED(latitude, longitude) =>
      ReasonReact.UpdateWithSideEffects(
        {...state, data: LOADING_SHOPS},
        (
          self =>
            Js.Promise.(
              fetch_shops(~latitude, ~longitude)
              |> then_(Fetch.Response.json)
              |> then_(json => json |> Decoder.shops |> resolve)
              |> then_(resp => {
                   self.send(AC_SHOPS_LOADED(resp.shops));
                   resolve(resp.shops);
                 })
              |> catch(err => {
                   Js.log(("error", err));
                   self.send(AC_LOADING_SHOPS_FAILED);
                   reject(Js.Exn.raiseError("failed to load shops"));
                 })
              |> ignore
            )
        )
      )
    | AC_SHOPS_LOADED(shops) =>
      ReasonReact.Update({...state, data: SHOPS_LOADED(shops)})
    | AC_LOADING_SHOPS_FAILED =>
      ReasonReact.Update({...state, data: LOADING_SHOPS_FAILED})
    | AC_CHANGE_SELECTED_TAB(selected_tab) =>
      ReasonReact.Update({...state, selected_tab})
    | AC_SHOP_LIKED(shop_id) =>
      switch state.data {
      | SHOPS_LOADED(shops) =>
        let liked =
          shops.near |> List.find((shop: shop) => shop.id == shop_id);
        let near =
          shops.near |> List.filter((shop: shop) => shop.id != shop_id);
        let preferred = shops.preferred |> List.append([liked]);
        ReasonReact.Update({...state, data: SHOPS_LOADED({near, preferred})});
      | _ => ReasonReact.NoUpdate
      }
    | _ => ReasonReact.NoUpdate
    },
  didMount: self => {
    let options: Geolocation.options = {
      enableHighAccuracy: true,
      timeout: 3000,
      maximumAge: 0
    };
    Geolocation.getCurrentPosition(
      options,
      ~success=
        position =>
          self.send(
            AC_LOCATION_LOADED(
              position##coords##latitude,
              position##coords##longitude
            )
          ),
      ~error=error => Js.log(("error", error))
    );
  },
  render: self =>
    <div className="content-wrapper">
      <Header
        changeSelectedTab=(
          (id: string, _) => self.send(AC_CHANGE_SELECTED_TAB(id))
        )
        selectedTab=self.state.selected_tab
        tabs=[
          {id: "near", text: "Near By Shops"},
          {id: "preferred", text: "My Preferred Shops"}
        ]
      />
      (
        switch self.state.data {
        | LOADING_LOCATION =>
          <p> (ReasonReact.string("getting your location")) </p>
        | LOADING_SHOPS => <p> (ReasonReact.string("loading shops")) </p>
        | SHOPS_LOADED(shops) =>
          switch self.state.selected_tab {
          | "near" =>
            <div className="shops-wrapper">
              <ShopsList
                data=shops.near
                btns=["dislike", "like"]
                clickHandler=(
                  (shop_id, btn_id, _) =>
                    clickHandler(self.send, shop_id, btn_id)
                )
              />
            </div>
          | "preferred" =>
            <div className="shops-wrapper">
              <ShopsList
                data=shops.preferred
                btns=["remove"]
                clickHandler=(
                  (shop_id, btn_id, _) =>
                    clickHandler(self.send, shop_id, btn_id)
                )
              />
            </div>
          | _ => <div> (ReasonReact.string("not a valid tab")) </div>
          }
        | LOADING_SHOPS_FAILED =>
          <div> (ReasonReact.string("failed to load shops")) </div>
        | _ => <div> (ReasonReact.string("else")) </div>
        }
      )
    </div>
};